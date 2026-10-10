#include "my_table.h"
#include "shape.h"
#include <windows.h>
#include <commctrl.h>
#include <string>

#pragma comment(lib, "comctl32.lib")

namespace
{
    constexpr int DELETE_BUTTON_ID = 1001;
    constexpr int BUTTON_WIDTH = 145;
    constexpr int BUTTON_HEIGHT = 28;
    constexpr int BUTTON_MARGIN = 10;
}

void MyTable::Show(HWND hParent)
{
    if (hWnd && IsWindow(hWnd))
    {
        ShowWindow(hWnd, SW_SHOW);
        SetForegroundWindow(hWnd);
        return;
    }

    this->hParent = hParent;

    const wchar_t CLASS_NAME[] = L"MyTableWindow";

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;

    RegisterClassExW(&wc);

    hWnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Таблиця фігур",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 650, 350,
        hParent,
        nullptr,
        GetModuleHandleW(nullptr),
        this);

    if (hWnd)
        ShowWindow(hWnd, SW_SHOW);
}

LRESULT CALLBACK MyTable::WindowProc(
    HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    MyTable* self = reinterpret_cast<MyTable*>(
        GetWindowLongPtrW(hWnd, GWLP_USERDATA));

    if (message == WM_NCCREATE)
    {
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<MyTable*>(cs->lpCreateParams);

        SetWindowLongPtrW(
            hWnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(self));
    }

    if (!self)
        return DefWindowProcW(hWnd, message, wParam, lParam);

    switch (message)
    {
    case WM_CREATE:
    {
        self->hList = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            WC_LISTVIEWW,
            L"",
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
            0, 0, 650, 270,
            hWnd,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr);

        ListView_SetExtendedListViewStyle(
            self->hList,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        const wchar_t* names[] = {
            L"Тип фігури", L"X1", L"Y1", L"X2", L"Y2"
        };

        for (int i = 0; i < 5; i++)
        {
            LVCOLUMNW col{};
            col.mask = LVCF_TEXT | LVCF_WIDTH;
            col.pszText = const_cast<LPWSTR>(names[i]);
            col.cx = (i == 0) ? 180 : 100;

            ListView_InsertColumn(self->hList, i, &col);
        }

        self->hDeleteButton = CreateWindowExW(
            0,
            L"BUTTON",
            L"Видалити вибране",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            BUTTON_MARGIN, 275,
            BUTTON_WIDTH, BUTTON_HEIGHT,
            hWnd,
            (HMENU)(INT_PTR)DELETE_BUTTON_ID,
            GetModuleHandleW(nullptr),
            nullptr);

        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == DELETE_BUTTON_ID &&
            HIWORD(wParam) == BN_CLICKED)
        {
            if (self->deleteCallback)
                self->deleteCallback(self->callbackContext);

            return 0;
        }
        break;

    case WM_NOTIFY:
    {
        LPNMHDR header = reinterpret_cast<LPNMHDR>(lParam);

        if (header->hwndFrom == self->hList &&
            header->code == LVN_ITEMCHANGED)
        {
            LPNMLISTVIEW info =
                reinterpret_cast<LPNMLISTVIEW>(lParam);

            if ((info->uChanged & LVIF_STATE) &&
                (info->uNewState & LVIS_SELECTED) &&
                !(info->uOldState & LVIS_SELECTED))
            {
                LVITEMW item{};
                item.mask = LVIF_PARAM;
                item.iItem = info->iItem;

                if (ListView_GetItem(self->hList, &item) &&
                    self->selectionCallback)
                {
                    self->selectionCallback(
                        static_cast<int>(item.lParam),
                        self->callbackContext);
                }
            }
        }

        return 0;
    }

    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        if (self->hList)
        {
            int listHeight = height - BUTTON_HEIGHT - 3 * BUTTON_MARGIN;

            if (listHeight < 0)
                listHeight = 0;

            MoveWindow(
                self->hList,
                0, 0,
                width,
                listHeight,
                TRUE);
        }

        if (self->hDeleteButton)
        {
            MoveWindow(
                self->hDeleteButton,
                BUTTON_MARGIN,
                height - BUTTON_HEIGHT - BUTTON_MARGIN,
                BUTTON_WIDTH,
                BUTTON_HEIGHT,
                TRUE);
        }

        return 0;
    }

    case WM_DESTROY:
        self->hWnd = nullptr;
        self->hList = nullptr;
        self->hDeleteButton = nullptr;
        return 0;
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

void MyTable::Close()
{
    if (hWnd && IsWindow(hWnd))
        DestroyWindow(hWnd);
}

void MyTable::SetSelectionCallback(
    SelectionCallback callback, void* context)
{
    selectionCallback = callback;
    callbackContext = context;
}

void MyTable::SetDeleteCallback(
    DeleteCallback callback, void* context)
{
    deleteCallback = callback;
    callbackContext = context;
}

void MyTable::Update(Shape** shapes, int count)
{
    if (!hList || !IsWindow(hList))
        return;

    ListView_DeleteAllItems(hList);

    for (int i = 0; i < count; i++)
    {
        if (!shapes[i])
            continue;

        LVITEMW item{};
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = ListView_GetItemCount(hList);
        item.lParam = i;
        item.pszText = const_cast<LPWSTR>(shapes[i]->GetType());

        int row = ListView_InsertItem(hList, &item);

        std::wstring x1 = std::to_wstring(shapes[i]->GetX1());
        std::wstring y1 = std::to_wstring(shapes[i]->GetY1());
        std::wstring x2 = std::to_wstring(shapes[i]->GetX2());
        std::wstring y2 = std::to_wstring(shapes[i]->GetY2());

        ListView_SetItemText(
            hList, row, 1, const_cast<LPWSTR>(x1.c_str()));
        ListView_SetItemText(
            hList, row, 2, const_cast<LPWSTR>(y1.c_str()));
        ListView_SetItemText(
            hList, row, 3, const_cast<LPWSTR>(x2.c_str()));
        ListView_SetItemText(
            hList, row, 4, const_cast<LPWSTR>(y2.c_str()));
    }
}

bool MyTable::IsOpen() const
{
    return hWnd && IsWindow(hWnd);
}