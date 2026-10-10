#pragma once
#include <windows.h>
#include "my_table.h"

class Shape;
class ShapeEditor;

class ShapeObjectsEditor
{
private:
    static ShapeObjectsEditor* instance;

    ShapeObjectsEditor();
    ~ShapeObjectsEditor();

    ShapeObjectsEditor(const ShapeObjectsEditor&) = delete;
    ShapeObjectsEditor& operator=(const ShapeObjectsEditor&) = delete;

    HWND mainWindow = nullptr;
    int selectedShapeIndex = -1;

    static void OnTableSelection(int index, void* context);
    static void OnDeleteShape(void* context);

    void DeleteSelectedShape();

protected:
    Shape** pcshape;
    int capacity;
    int currentIndex;
    ShapeEditor* pse = nullptr;
    int currentTool = 0;
    MyTable table;

public:
    static ShapeObjectsEditor* GetInstance();
    static void DeleteInstance();

    void AddShape(Shape* shape);
    void ToggleTable(HWND hWnd);

    void StartPointEditor();
    void StartLineEditor();
    void StartRectEditor();
    void StartEllipseEditor();
    void StartLineWithCirclesEditor();
    void StartCubeFrameEditor();

    void OnLBdown(HWND hWnd);
    void OnLBup(HWND hWnd);
    void OnMouseMove(HWND hWnd);
    void OnPaint(HWND hWnd);
    void OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam);

    bool SaveToFile(const wchar_t* filename);
    bool LoadFromFile(const wchar_t* filename);
};

using MyEditor = ShapeObjectsEditor;