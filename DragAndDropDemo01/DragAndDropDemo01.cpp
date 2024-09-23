// DragAndDropDemo01.cpp : Defines the entry point for the application.
//

#include "Common.h"
#include "DragAndDropDemo01.h"
#include "MyWindowsOleDropTarget.h"
#include <iostream>

static constexpr SIZE_T MAX_LOADSTRING = 128;

static HINSTANCE hInst;
static WCHAR     szTitle[MAX_LOADSTRING];
static WCHAR     szWindowClass[MAX_LOADSTRING];

static ATOM             MyRegisterClass(HINSTANCE);
static BOOL             InitInstance(HINSTANCE, int);
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(
       _In_     HINSTANCE hInstance,
    UU _In_opt_ HINSTANCE hPrevInstance,
    UU _In_     LPWSTR    lpCmdLine,
       _In_     int       nShowCmd)
{
    util::OpenConsoleWindow();
    if (auto res = ::OleInitialize(nullptr); FAILED(res))
        util::DumpErrorAndCrash(res);

    // Initialize global strings
    ::LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    ::LoadStringW(hInstance, IDC_DRAGANDDROPDEMO01, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nShowCmd))
        return FALSE;

    HACCEL hAccelTable = ::LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(IDC_DRAGANDDROPDEMO01));
    MSG    msg;

    // Main message loop:
    while (::GetMessageW(&msg, nullptr, 0, 0)) {
        if (!::TranslateAcceleratorW(msg.hwnd, hAccelTable, &msg)) {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
    }

    ::OleUninitialize();
    return static_cast<int>(msg.wParam);
}

// Registers the window class.
static ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {
        .cbSize        = sizeof wcex,
        .style         = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc   = &WndProc,
        .cbClsExtra    = 0,
        .cbWndExtra    = 0,
        .hInstance     = hInstance,
        .hIcon         = ::LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_DRAGANDDROPDEMO01)),
        .hCursor       = ::LoadCursorW(nullptr, IDC_ARROW),
        .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
        .lpszMenuName  = MAKEINTRESOURCEW(IDC_DRAGANDDROPDEMO01),
        .lpszClassName = szWindowClass,
    };
    wcex.hIconSm = ::LoadIconW(wcex.hInstance, MAKEINTRESOURCEW(IDI_SMALL));
    return ::RegisterClassExW(&wcex);
}

// Message handler for about box.
static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, UU LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            ::EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        [[fallthrough]];
    default:
        return FALSE;
    }
}

/****************************************************************************************/

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
static BOOL InitInstance(HINSTANCE hInstance, int nShowCmd)
{
    hInst = hInstance;
    HWND hWnd = ::CreateWindowExW(
        0U, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT, 0U, CW_USEDEFAULT, 0U, 
        nullptr, nullptr, hInstance, nullptr
    );
    if (!hWnd)
        return FALSE;
    ::ShowWindow(hWnd, nShowCmd);
    ::UpdateWindow(hWnd);
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
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static MyWindowsOleDropTarget *oleDropTarget;

    switch (message) {
    case WM_COMMAND: {
        // Parse the menu selections:
        switch (int wmId = LOWORD(wParam)) {
        case IDM_ABOUT:
            ::DialogBoxParamW(hInst, MAKEINTRESOURCEW(IDD_ABOUTBOX), hWnd, &About, 0);
            break;
        case IDM_EXIT:
            ::DestroyWindow(hWnd);
            break;
        default:
            return ::DefWindowProcW(hWnd, message, wParam, lParam);
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        ::BeginPaint(hWnd, &ps);
        ::EndPaint(hWnd, &ps);
        break;
    }

    case WM_CREATE: {
        oleDropTarget = new MyWindowsOleDropTarget(hWnd);
        HRESULT res   = ::RegisterDragDrop(hWnd, oleDropTarget);
        if (FAILED(res))
            util::DumpErrorAndCrash(res);
        break;
    }

    case WM_DESTROY: {
        HRESULT res = ::RevokeDragDrop(hWnd);
        if (FAILED(res))
            std::wcerr << util::GetErrorMessage(res) << L'\n';
        oleDropTarget->Release();
        ::PostQuitMessage(0);
        break;
    }

    default:
        return ::DefWindowProcW(hWnd, message, wParam, lParam);
    }

    return 0;
}
