#pragma once
#include <windows.h>
#include <commctrl.h>

class Shape;

class MyTable
{
private:
    HWND hWnd = nullptr;
    HWND hList = nullptr;
    HWND hDeleteButton = nullptr;
    HWND hParent = nullptr;

    using SelectionCallback = void (*)(int, void*);
    using DeleteCallback = void (*)(void*);

    SelectionCallback selectionCallback = nullptr;
    DeleteCallback deleteCallback = nullptr;
    void* callbackContext = nullptr;

    static LRESULT CALLBACK WindowProc(
        HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
    MyTable() = default;

    void Show(HWND hParent);
    void Close();
    void Update(Shape** shapes, int count);
    bool IsOpen() const;

    void SetSelectionCallback(
        SelectionCallback callback, void* context);

    void SetDeleteCallback(
        DeleteCallback callback, void* context);
};