// Lab2_oop.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include <commdlg.h>
#pragma comment(lib, "Comdlg32.lib")
#include "Resource.h"
#include "Lab2_oop.h"
#include "shape_editor.h"
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#include "my_table.h"

#define MAX_LOADSTRING 100
#define IDC_TOOLBAR 1001

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

MyEditor* pEditor = nullptr;
MyTable table;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void UpdateWindowTitle(HWND hWnd, const wchar_t* shapeName)
{
    wchar_t title[150];
    swprintf_s(title, L"Lab3_oop - Поточна фігура: %s", shapeName);
    SetWindowTextW(hWnd, title);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LAB2OOP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB2OOP));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB2OOP));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LAB2OOP);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    pEditor = MyEditor::GetInstance();

    InitCommonControls();

    TBBUTTON tbb[6] =
    {
        { 0, ID_POINT,        TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0 },
        { 1, ID_LINE,         TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0 },
        { 2, ID_RECT,         TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0 },
        { 3, ID_ELLIPSE,      TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0 },
        { 4, ID_LINE_CIRCLES, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0 },
        { 5, ID_CUBEFRAME,    TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0 }
    };

    HWND hWndToolbar = CreateToolbarEx(
        hWnd,
        WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
        IDC_TOOLBAR,
        6,
        hInst,
        IDB_TOOLBAR,
        tbb,
        6,
        32, 32,
        32, 32,
        sizeof(TBBUTTON)
    );

    SendMessage(hWndToolbar, TB_SETBUTTONSIZE, 0, (LPARAM)MAKELONG(40, 40));
    SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case ID_POINT:
            if (pEditor) pEditor->StartPointEditor();
            UpdateWindowTitle(hWnd, L"Крапка");
            break;
        case ID_LINE:
            if (pEditor) pEditor->StartLineEditor();
            UpdateWindowTitle(hWnd, L"Лінія");
            break;
        case ID_RECT:
            if (pEditor) pEditor->StartRectEditor();
            UpdateWindowTitle(hWnd, L"Прямокутник");
            break;
        case ID_ELLIPSE:
            if (pEditor) pEditor->StartEllipseEditor();
            UpdateWindowTitle(hWnd, L"Еліпс");
            break;
        case ID_LINE_CIRCLES:
            if (pEditor) pEditor->StartLineWithCirclesEditor();
            UpdateWindowTitle(hWnd, L"Лінія з кружечками");
            break;
        case ID_CUBEFRAME:
            if (pEditor) pEditor->StartCubeFrameEditor();
            UpdateWindowTitle(hWnd, L"Каркас куба");
            break;
        case ID_FILE_SAVE:
            if (pEditor)
            {
                if (pEditor->SaveToFile(L"shapes.csv"))
                    MessageBoxW(hWnd, L"Файл збережено!",
                        L"Збереження", MB_OK | MB_ICONINFORMATION);
                else
                    MessageBoxW(hWnd, L"Не вдалося зберегти файл.",
                        L"Помилка", MB_OK | MB_ICONERROR);
            }
            break;
        case ID_FILE_LOAD:
        {
            wchar_t filename[MAX_PATH] = L"";

            OPENFILENAMEW ofn{};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFilter =
                L"CSV-файли (*.csv)\0*.csv\0Усі файли (*.*)\0*.*\0";
            ofn.lpstrFile = filename;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrDefExt = L"csv";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

            if (GetOpenFileNameW(&ofn))
            {
                if (!pEditor->LoadFromFile(filename))
                    MessageBoxW(hWnd, L"Помилка завантаження CSV.",
                        L"Помилка", MB_OK | MB_ICONERROR);
            }

            break;
        }
        case ID_OBJECTS_TABLE:
            if (pEditor)
                pEditor->ToggleTable(hWnd);
            return 0;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        if (pEditor) pEditor->OnPaint(hWnd);
    }
    break;
    case WM_LBUTTONDOWN:
        if (pEditor) pEditor->OnLBdown(hWnd);
        break;
    case WM_LBUTTONUP:
        if (pEditor) pEditor->OnLBup(hWnd);
        break;
    case WM_MOUSEMOVE:
        if (pEditor) pEditor->OnMouseMove(hWnd);
        break;
    case WM_NOTIFY:
        if (pEditor) pEditor->OnNotify(hWnd, wParam, lParam);
        break;
    case WM_DESTROY:
        MyEditor::DeleteInstance();
        pEditor = nullptr;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
  