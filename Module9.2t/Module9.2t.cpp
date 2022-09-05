//#include <iostream>
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include "Delegate.h"
#include <thread>
#include <mutex>
#include "Timer.cpp"
#include <random>
#include <algorithm>

using namespace Gdiplus;
#pragma comment(lib, "Gdiplus.lib")

RECT rect;
std::mutex g_lock;
HDC hdcMem;
HBITMAP hbmMem;
HANDLE hOld;
int DoOnce = 0;

void RedrawGame(HDC hdc);
void InitializeGame();
void DrawIm(HDC hdc, const WCHAR* Path);
void DrawActorG(HDC hdc, const WCHAR* Path, int x1, int y1, int sizex, int sizey);
void EnemyDead(int Number);
void DrawCollisions(HDC hdc, int x, int y, int sizex, int sizey);

int f(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
    int left = max(x1, x3);
    int top = min(y2, y4);
    int right = min(x2, x4);
    int bottom = max(y1, y3);

    int width = right - left;
    int height = bottom - top;

    if (width < 0 || height < 0) return 0;

    return width * height;
}

bool Intersects(RECT r1, RECT r2)
{
    if (r1.left > r2.right || r1.right < r2.left || r1.top < r2.bottom || r1.bottom > r2.top) return false;
    return true;
}

int GetRandomIntInRange(const int min, const int max)
{
    static std::default_random_engine gen(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(gen);
}

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
    RECT GetActorCollision() { return ActorCollision; }
    int GetX() { return x; }
    int GetY() { return y; }

protected:
    int x, y;
    int sizex, sizey;
    const WCHAR* ActPath;
    RECT ActorCollision;
};

class HeroCar : public Actor, public Observer
{
public:
    HeroCar(int x1, int y1, const WCHAR* Path, Subject* mod, int div) : Actor(x1, y1, Path), Observer(mod, div) { ActPath = Path; }
    virtual void Move(int Direction) override
    {
        switch (Direction)
        {
            case (2): break;
            case (4):
                x = x + (rect.left - rect.right) / 3;
                /*ActorCollision.left = ActorCollision.left + (rect.left - rect.right) / 3;
                ActorCollision.right = ActorCollision.right + (rect.left - rect.right) / 3;*/

                ActorCollision.left = x - 10;
                ActorCollision.right = x + 10;
                ActorCollision.top = y - 10;
                ActorCollision.bottom = y + 10;

                break;
            case (6):
                x = x - (rect.left - rect.right) / 3;
                /*ActorCollision.left = ActorCollision.left - (rect.left - rect.right) / 3;
                ActorCollision.right = ActorCollision.right - (rect.left - rect.right) / 3;*/

                ActorCollision.left = x - 10;
                ActorCollision.right = x + 10;
                ActorCollision.top = y - 10;
                ActorCollision.bottom = y + 10;
                break;
        }
    }

    void update()
    {
        // 6. "Вытягивание" интересующей информации
        int v = getSubject()->getVal(), d = getDivisor();
        Move(v);
    }

    virtual void DrawActor(HDC hdc) override
    {
        DrawActorG(hdc, ActPath, x, y, sizex, sizey);
        DrawCollisions(hdc, x, y, sizex, sizey);
    }

    Observer* OnMove;
};

class EnemyCar : public Actor
{
public:
    EnemyCar(int x1, int y1, const WCHAR* Path, int DeltaX, int DeltaY, int EnemyNumber, int AbroadInt) : Actor(x1, y1, Path)
    {
        ActPath = Path;
        this->DeltaX = DeltaX;
        this->DeltaY = DeltaY;
        this->EnemyNumber = EnemyNumber;
        Abroad = AbroadInt == 0 ? true : false;
    }
    virtual void DrawActor(HDC hdc) override
    {
        DrawActorG(hdc, ActPath, x, y, sizex, sizey);
        DrawCollisions(hdc, x, y, sizex, sizey);
    }

    virtual void Move(int Direction) override
    {
        if (Abroad)
        {
            y = y + DeltaY;
            x = x + DeltaX;
            ActorCollision.left = ActorCollision.left + DeltaX;
            ActorCollision.right = ActorCollision.right + DeltaX;
            ActorCollision.top = ActorCollision.top + DeltaY;
            ActorCollision.bottom = ActorCollision.bottom + DeltaY;
        }
        else
        {
            y = y + DeltaY;
            x = x - DeltaX;
            ActorCollision.left = ActorCollision.left - DeltaX;
            ActorCollision.right = ActorCollision.right - DeltaX;
            ActorCollision.top = ActorCollision.top + DeltaY;
            ActorCollision.bottom = ActorCollision.bottom + DeltaY;
        }

        if (x > rect.right - 250 || x < 0) Abroad = !Abroad;

        if (y > rect.bottom) EnemyDead(EnemyNumber);
    }

private:
    bool Abroad = false;
    int DeltaX, DeltaY;
    int EnemyNumber;
};

bool intersects(RECT a, RECT b)
{
    return (a.top < b.bottom || a.bottom > b.top || a.right < b.left || a.left > b.right);
}

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

void DrawCollisions(HDC hdc, int x1, int y1, int sizex, int sizey)
{
    Graphics graphics(hdc);

    // Create a Pen object.
    Pen blackPen(Color(255, 0, 0, 0), 3);

    // Create a RectF object.
    RectF rect(x1, y1, sizex, sizey);

    // Draw rect.
    graphics.DrawRectangle(&blackPen, rect);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    ULONG_PTR token;
    GdiplusStartupInput input = {0};
    input.GdiplusVersion = 1;
    GdiplusStartup(&token, &input, NULL);

    const wchar_t CLASS_NAME[] = L"RunnerGame";
    WNDCLASS wc = {};
    wc.lpfnWndProc = &WindowProc;  // attach this callback procedure
    wc.hInstance = hInstance;      // handle to application instance
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = NULL;
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

HeroCar* Hero;
std::vector<EnemyCar*> Enemy;
Background* Back;
Subject OnMove;
int GoLeft = 0;
int GoRight = 0;

void EnemyDead(int Number)
{
    Enemy.erase(Enemy.begin() + Number);
    // std::vector<EnemyCar*>(Enemy).swap(Enemy);
}

void Update(HDC hdc, HWND hwnd)
{
    g_lock.lock();

    GetWindowRect(hwnd, &rect);
    Back->DrawBackground(hdc);
    Hero->DrawActor(hdc);
    for (auto CurrentEnemy : Enemy)
    {
        CurrentEnemy->DrawActor(hdc);
    }

    // Здесь рисуем в hdcMem
    // RedrawGame(hdcMem);

    g_lock.unlock();
}

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
        case WM_TIMER:
            InvalidateRect(hwnd, NULL, FALSE);
            for (auto CurrentEnemy : Enemy)
            {
                if (CurrentEnemy == NULL) break;
                CurrentEnemy->Move(0);

                RectF rect1(Hero->GetX(), Hero->GetY(), 100, 100);
                RectF rect2(CurrentEnemy->GetX(), CurrentEnemy->GetY(), 100, 100);
                if (rect1.IntersectsWith(rect2)) Enemy.erase(Enemy.begin(), Enemy.end());
            }
                
            break;
        case WM_DESTROY: PostQuitMessage(0); return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Создаем off-screen DC для двойной буферизации
            hdcMem = CreateCompatibleDC(hdc);
            hbmMem = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);
            hOld = SelectObject(hdcMem, hbmMem);

            std::thread VisualThread(Update, hdcMem, hwnd);  //Создаём визуальный поток и отправляем ему дескриптор в памяти для отрисовки
            VisualThread.join();

            // Выводим построенное  изображение и памяти на экран
            BitBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hdcMem, 0, 0, SRCCOPY);

            // VisualThread.join();

            // Освобождаем память
            SelectObject(hdcMem, hOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);

            // RedrawGame(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_KEYDOWN:
            switch (wParam)
            {

                case VK_LEFT:
                    if (GoLeft == 1) break;
                    OnMove.setVal(4);
                    GoLeft++;
                    GoRight--;
                    break;
                case VK_RIGHT:
                    if (GoRight == 1) break;
                    OnMove.setVal(6);
                    GoLeft--;
                    GoRight++;
                    break;
            }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void test1(void)
{
    Enemy.push_back(new EnemyCar(200, 0, L"C:/Users/ADMIN/Downloads/Enemy.png", GetRandomIntInRange(1, 5), GetRandomIntInRange(1, 10), 0,
        GetRandomIntInRange(0, 1)));
    return;
}

void ThreadSpawn()
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(GetRandomIntInRange(2000, 5000)));
        later later_test1(1000, false, &test1);
    }
}

void InitializeGame()
{
    Hero = new HeroCar(200, 350, L"C:/Users/ADMIN/Downloads/Car.png", &OnMove, 0);
    Back = new Background(L"C:/Users/ADMIN/Downloads/Back.jpg");

    std::thread SpawnThread(ThreadSpawn);
    SpawnThread.detach();
}