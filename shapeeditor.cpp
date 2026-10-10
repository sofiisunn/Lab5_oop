#include "shape_editor.h"
#include "editor.h"
#include "Resource.h"
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#include <fstream>
#include <cstdio>
#include <string>
#include <cwchar>
#include <cstdlib>
#include <sstream>
#include <vector>

ShapeObjectsEditor* ShapeObjectsEditor::instance = nullptr;

ShapeObjectsEditor* ShapeObjectsEditor::GetInstance()
{
    if (instance == nullptr)
        instance = new ShapeObjectsEditor();

    return instance;
}

void ShapeObjectsEditor::DeleteInstance()
{
    delete instance;
    instance = nullptr;
}

ShapeObjectsEditor::ShapeObjectsEditor()
{
    capacity = 121;
    pcshape = new Shape * [capacity];
    currentIndex = 0;
    pse = nullptr;
    currentTool = -1;
    for (int i = 0; i < 121; i++)
    {
        pcshape[i] = nullptr;
    }
}

ShapeObjectsEditor::~ShapeObjectsEditor()
{
    delete pse;
    for (int i = 0; i < currentIndex; i++)
    {
        delete pcshape[i];
    }
    delete[] pcshape;
}

void ShapeObjectsEditor::AddShape(Shape* shape)
{
    if (currentIndex < capacity && shape != nullptr)
    {
        pcshape[currentIndex] = shape;
        currentIndex++;
        table.Update(pcshape, currentIndex);
    }
}

bool ShapeObjectsEditor::SaveToFile(const wchar_t* filename)
{
    wchar_t path[MAX_PATH];

    DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
        return false;

    wchar_t* slash = wcsrchr(path, L'\\');
    if (slash == nullptr)
        return false;

    *(slash + 1) = L'\0';

    if (wcscat_s(path, MAX_PATH, filename) != 0)
        return false;

    FILE* file = nullptr;

    if (_wfopen_s(&file, path, L"wb") != 0 || file == nullptr)
    {
        MessageBoxW(nullptr, path, L"Не вдалося відкрити файл", MB_OK | MB_ICONERROR);
        return false;
    }

    const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
    bool success = fwrite(bom, 1, 3, file) == 3;

    for (int i = 0; success && i < currentIndex; i++)
    {
        if (!pcshape[i])
            continue;

        wchar_t line[512];

        int len = swprintf_s(line, 512, L"%ls;%d;%d;%d;%d\r\n",
            pcshape[i]->GetType(),
            pcshape[i]->GetX1(), pcshape[i]->GetY1(),
            pcshape[i]->GetX2(), pcshape[i]->GetY2());

        if (len < 0)
        {
            success = false;
            break;
        }

        int size = WideCharToMultiByte(CP_UTF8, 0, line, len,
            nullptr, 0, nullptr, nullptr);

        if (size <= 0)
        {
            success = false;
            break;
        }

        std::string utf8(size, '\0');

        if (WideCharToMultiByte(CP_UTF8, 0, line, len,
            &utf8[0], size, nullptr, nullptr) == 0 ||
            fwrite(utf8.data(), 1, size, file) != (size_t)size)
        {
            success = false;
        }
    }

    if (fclose(file) != 0)
        success = false;

    if (success)
        MessageBoxW(nullptr, path, L"CSV збережено тут", MB_OK);
    else
        MessageBoxW(nullptr, L"Помилка запису у файл", L"Помилка", MB_OK | MB_ICONERROR);

    return success;
}

bool ShapeObjectsEditor::LoadFromFile(const wchar_t* filename)
{
    FILE* file = nullptr;
    wchar_t path[MAX_PATH] = L"";

    DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    wchar_t* slash = wcsrchr(path, L'\\');
    if (length > 0 && length < MAX_PATH && slash != nullptr)
    {
        *(slash + 1) = L'\0';
        wcscat_s(path, MAX_PATH, filename);
        _wfopen_s(&file, path, L"rb");
    }
    if (file == nullptr)
        _wfopen_s(&file, filename, L"rb");

    if (file == nullptr)
    {
        MessageBoxW(nullptr, filename, L"Не вдалося відкрити файл", MB_OK | MB_ICONERROR);
        return false;
    }

    unsigned char bom[3] = {};
    if (fread(bom, 1, 3, file) != 3 || bom[0] != 0xEF || bom[1] != 0xBB || bom[2] != 0xBF)
        rewind(file);

    std::vector<Shape*> shapes;
    char line[1024];
    int lineNumber = 0;
    bool ok = true;

    while (ok && fgets(line, sizeof(line), file))
    {
        lineNumber++;

        wchar_t wide[1024];
        if (MultiByteToWideChar(CP_UTF8, 0, line, -1, wide, 1024) <= 0)
        {
            ok = false;
            break;
        }

        size_t len = wcslen(wide);
        while (len > 0 && (wide[len - 1] == L'\r' || wide[len - 1] == L'\n'))
            wide[--len] = L'\0';
        if (len == 0)
            continue;

        wchar_t* context = nullptr;
        wchar_t* type = wcstok_s(wide, L";", &context);
        long coords[4] = {};
        bool valid = (type != nullptr);

        for (int i = 0; valid && i < 4; i++)
        {
            wchar_t* token = wcstok_s(nullptr, L";", &context);
            if (token == nullptr)
            {
                valid = false;
                break;
            }
            wchar_t* end = nullptr;
            coords[i] = wcstol(token, &end, 10);
            if (end == token || *end != L'\0')
                valid = false;
        }

        Shape* s = nullptr;
        if (valid)
        {
            if (wcscmp(type, L"Крапка") == 0)                    s = new PointShape();
            else if (wcscmp(type, L"Лінія") == 0)                s = new LineShape();
            else if (wcscmp(type, L"Прямокутник") == 0)          s = new RectShape();
            else if (wcscmp(type, L"Еліпс") == 0)                s = new EllipseShape();
            else if (wcscmp(type, L"Лінія з кружечками") == 0)   s = new LineWithCirclesShape();
            else if (wcscmp(type, L"Каркас куба") == 0)          s = new CubeFrameShape();
        }

        if (s == nullptr || shapes.size() >= (size_t)capacity)
        {
            delete s;
            ok = false;
            break;
        }

        s->Set(coords[0], coords[1], coords[2], coords[3]);
        shapes.push_back(s);
    }

    if (ferror(file))
        ok = false;
    fclose(file);

    if (!ok)
    {
        for (Shape* s : shapes)
            delete s;

        wchar_t msg[100];
        swprintf_s(msg, L"Помилка у рядку %d файлу", lineNumber);
        MessageBoxW(nullptr, msg, L"Не вдалося завантажити", MB_OK | MB_ICONERROR);
        return false;
    }

    for (int i = 0; i < currentIndex; i++)
        delete pcshape[i];

    currentIndex = 0;
    selectedShapeIndex = -1;

    for (Shape* s : shapes)
        pcshape[currentIndex++] = s;

    for (int i = currentIndex; i < capacity; i++)
        pcshape[i] = nullptr;

    table.Update(pcshape, currentIndex);

    HWND hwnd = mainWindow ? mainWindow : GetActiveWindow();
    if (hwnd)
        InvalidateRect(hwnd, nullptr, TRUE);

    return true;
}
void ShapeObjectsEditor::ToggleTable(HWND hWnd)
{
    if (table.IsOpen())
    {
        table.Close();
    }
    else
    {
        mainWindow = hWnd;

        table.SetSelectionCallback(OnTableSelection, this);
        table.SetDeleteCallback(OnDeleteShape, this);

        table.Show(hWnd);
    }

    table.Update(pcshape, currentIndex);
}

void ShapeObjectsEditor::OnTableSelection(int index, void* context)
{
    ShapeObjectsEditor* editor =
        static_cast<ShapeObjectsEditor*>(context);

    if (!editor)
        return;

    editor->selectedShapeIndex = index;

    if (editor->mainWindow)
        InvalidateRect(editor->mainWindow, nullptr, TRUE);
}

void ShapeObjectsEditor::OnDeleteShape(void* context)
{
    ShapeObjectsEditor* editor =
        static_cast<ShapeObjectsEditor*>(context);

    if (editor)
        editor->DeleteSelectedShape();
}

void ShapeObjectsEditor::DeleteSelectedShape()
{
    if (selectedShapeIndex < 0 ||
        selectedShapeIndex >= currentIndex ||
        !pcshape[selectedShapeIndex])
        return;

    delete pcshape[selectedShapeIndex];

    for (int i = selectedShapeIndex; i < currentIndex - 1; i++)
        pcshape[i] = pcshape[i + 1];

    pcshape[--currentIndex] = nullptr;
    selectedShapeIndex = -1;

    table.Update(pcshape, currentIndex);

    if (mainWindow)
        InvalidateRect(mainWindow, nullptr, TRUE);
}

void ShapeObjectsEditor::OnLBdown(HWND hWnd)
{
    if (pse) pse->OnLBdown(hWnd);
}

void ShapeObjectsEditor::OnLBup(HWND hWnd)
{
    if (pse) pse->OnLBup(hWnd);
}

void ShapeObjectsEditor::OnMouseMove(HWND hWnd)
{
    if (pse) pse->OnMouseMove(hWnd);
}

void ShapeObjectsEditor::OnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    for (int i = 0; i < currentIndex; i++)
    {
        if (pcshape[i])
            pcshape[i]->Show(hdc);
    }

    if (selectedShapeIndex >= 0 &&
        selectedShapeIndex < currentIndex &&
        pcshape[selectedShapeIndex])
    {
        Shape* shape = pcshape[selectedShapeIndex];

        long x1 = shape->GetX1();
        long y1 = shape->GetY1();
        long x2 = shape->GetX2();
        long y2 = shape->GetY2();

        long left, right, top, bottom;

        if (wcscmp(shape->GetType(), L"Прямокутник") == 0 ||
            wcscmp(shape->GetType(), L"Каркас куба") == 0)
        {
            long dx = labs(x2 - x1);
            long dy = labs(y2 - y1);

            left = x1 - dx;
            right = x1 + dx;
            top = y1 - dy;
            bottom = y1 + dy;
        }
        else
        {
            left = (x1 < x2) ? x1 : x2;
            right = (x1 > x2) ? x1 : x2;
            top = (y1 < y2) ? y1 : y2;
            bottom = (y1 > y2) ? y1 : y2;
        }

        if (right - left < 8)
        {
            left -= 4;
            right += 4;
        }

        if (bottom - top < 8)
        {
            top -= 4;
            bottom += 4;
        }

        HPEN pen = CreatePen(PS_SOLID, 1, RGB(220, 100, 180));
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(
            hdc, GetStockObject(HOLLOW_BRUSH));

        Rectangle(
            hdc,
            (int)left - 3,
            (int)top - 3,
            (int)right + 4,
            (int)bottom + 4
        );

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    if (pse)
        pse->OnPaint(hdc);

    EndPaint(hWnd, &ps);
}
void ShapeObjectsEditor::StartPointEditor()
{
    delete pse;
    currentTool = 0;
    pse = new PointEditor(this);
}

void ShapeObjectsEditor::StartLineEditor()
{
    delete pse;
    currentTool = 1;
    pse = new LineEditor(this);
}

void ShapeObjectsEditor::StartRectEditor()
{
    delete pse;
    currentTool = 2;
    pse = new RectEditor(this);
}

void ShapeObjectsEditor::StartEllipseEditor()
{
    delete pse;
    currentTool = 3;
    pse = new EllipseEditor(this);
}

void ShapeObjectsEditor::StartLineWithCirclesEditor()
{
    delete pse;
    pse = new LineWithCirclesEditor(this);
}

void ShapeObjectsEditor::StartCubeFrameEditor()
{
    delete pse;
    pse = new CubeFrameEditor(this);
}

void ShapeObjectsEditor::OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pnmh = (LPNMHDR)lParam;
    if (pnmh->code == TTN_GETDISPINFO)
    {
        LPNMTTDISPINFO pDispInfo = (LPNMTTDISPINFO)lParam;
        switch (pDispInfo->hdr.idFrom)
        {
        case ID_POINT:
            pDispInfo->lpszText = (LPWSTR)L"Крапка";
            break;
        case ID_LINE:
            pDispInfo->lpszText = (LPWSTR)L"Лінія";
            break;
        case ID_RECT:
            pDispInfo->lpszText = (LPWSTR)L"Прямокутник";
            break;
        case ID_ELLIPSE:
            pDispInfo->lpszText = (LPWSTR)L"Еліпс";
            break;
        case ID_LINE_CIRCLES:
            pDispInfo->lpszText = (LPWSTR)L"Лінія з кружечками";
            break;
        case ID_CUBEFRAME:
            pDispInfo->lpszText = (LPWSTR)L"Каркас куба";
            break;
        }
    }
}

