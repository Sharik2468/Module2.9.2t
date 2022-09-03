#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>

using namespace Gdiplus;
#pragma comment(lib, "Gdiplus.lib")

RECT rect;

void DrawBackground(HDC hdc)
{
    Graphics graphics(hdc);
    // Create an Image object.
    Image image(L"C:/Users/ADMIN/Downloads/Back.jpg");
    // Create a Pen object.
    /*Pen pen(Color(255, 255, 0, 0), 2);*/
    // Draw the original source image.
    graphics.DrawImage(&image, 0, 0, rect.right - rect.left, rect.bottom - rect.top);
    // Create a Rect object that specifies the destination of the image.
    // Rect destRect(200, 50, 150, 75);
    //// Draw the rectangle that bounds the image.
    // graphics.DrawRectangle(&pen, destRect);
    //// Draw the image.
    // graphics.DrawImage(&image, destRect);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    ULONG_PTR token;
    GdiplusStartupInput input = {0};
    input.GdiplusVersion = 1;
    GdiplusStartup(&token, &input, NULL);

    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    WNDCLASS wc = {};
    wc.lpfnWndProc = &WindowProc;  // attach this callback procedure
    wc.hInstance = hInstance;      // handle to application instance
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);  // register wc
    // Create the window.
    HWND hwnd = CreateWindowEx(0,     // Optional window styles.
        CLASS_NAME,                   // Window class
        L"Learn to Program Windows",  // Window text
        WS_OVERLAPPEDWINDOW,          // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,

        NULL,       // Parent window
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd != NULL)
    {
        ShowWindow(hwnd, nCmdShow);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    GdiplusShutdown(token);
    return 0;
}

// callback procedure for this window, takes in all the window details
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE: SetTimer(hwnd, 1, 20, NULL); break;
        case WM_TIMER: InvalidateRect(hwnd, NULL, FALSE); break;
        case WM_DESTROY: PostQuitMessage(0); return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            GetWindowRect(hwnd, &rect);
            DrawBackground(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}