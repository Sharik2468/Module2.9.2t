#include <iostream>
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include "Delegate.h"
//#include <thread>
//#include <mutex>

using namespace Gdiplus;
#pragma comment(lib, "Gdiplus.lib")

RECT rect;
//std::mutex g_lock;

void RedrawGame(HDC hdc);
void InitializeGame();
void DrawIm(HDC hdc, const WCHAR* Path);
void DrawActorG(HDC hdc, const WCHAR* Path, int x1, int y1, int sizex, int sizey);


class Background
{
private:
    const WCHAR* BGPath;

public:
    void DrawBackground(HDC hdc) { DrawIm(hdc, BGPath); }
    Background(const WCHAR* Path) { BGPath = Path; }
};

class Actor
{
public:
    Actor(int x1, int y1, const WCHAR* Path)
    {
        x = x1;
        y = y1;
        sizex = 100;
        sizey = 100;
    }
    virtual void Move(int Direction){};
    virtual void DrawActor(HDC hdc){};

protected:
    int x, y;
    int sizex, sizey;
    const WCHAR* ActPath;
};

class HeroCar : public Actor
{
public:
    HeroCar(int x1, int y1, const WCHAR* Path) : Actor(x1, y1, Path) { ActPath = Path; }
    virtual void Move(int Direction) override
    {
        switch (Direction)
        {
            case (2): break;
            case (4): x = x - 1; break;
            case (6): x = x + 1; break;
        }
    }

    virtual void DrawActor(HDC hdc) override { DrawActorG(hdc, ActPath, x, y, sizex, sizey); }
};

class EnemyCar : public Actor
{
public:
    EnemyCar(int x1, int y1, const WCHAR* Path) : Actor(x1, y1, Path) {}
    virtual void Move(int Direction) override
    {
        switch (Direction)
        {
            case (2): break;
            case (4):  break;
            case (6):  break;
        }
    }
};

void DrawIm(HDC hdc, const WCHAR* Path)
{
    Graphics graphics(hdc);
    // Create an Image object.
    Image image(Path);
    // Draw the original source image.
    graphics.DrawImage(&image, 0, 0, rect.right - rect.left, rect.bottom - rect.top);
}

void DrawActorG(HDC hdc, const WCHAR* Path, int x1, int y1, int sizex, int sizey)
{
    Graphics graphics(hdc);
    // Create an Image object.
    Image image(Path);
    // Draw the original source image.
    graphics.DrawImage(&image, x1, y1, sizex, sizey);
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

    InitializeGame();

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

int DoOnce = 0;
HeroCar* Hero;
Background* Back;

// callback procedure for this window, takes in all the window details
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (DoOnce == 0)
    {
        DoOnce++;
    }
    switch (uMsg)
    {
        case WM_CREATE: SetTimer(hwnd, 1, 20, NULL); break;
        case WM_TIMER: InvalidateRect(hwnd, NULL, FALSE); break;
        case WM_DESTROY: PostQuitMessage(0); return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            //std::thread VisualThread(RedrawGame, hdc);
            GetWindowRect(hwnd, &rect);
            RedrawGame(hdc);
            EndPaint(hwnd, &ps);
            //VisualThread.join();
            return 0;
        }

        case WM_KEYDOWN:
            switch (wParam)
            {
                case VK_LEFT: Hero->Move(4); break;
                case VK_RIGHT: Hero->Move(6); break;
            }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void RedrawGame(HDC hdc)
{
    //g_lock.lock();
    

    Back->DrawBackground(hdc);
    Hero->DrawActor(hdc);

    
    //g_lock.unlock();
}

void InitializeGame()
{
    Hero = new HeroCar(250, 100, L"C:/Users/ADMIN/Downloads/Car.png");
    Back = new Background(L"C:/Users/ADMIN/Downloads/Back.jpg");
}