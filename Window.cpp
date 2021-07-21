#include "pch.h"
#include "Window.h"
#include "Renderer.h"
#include "Input.h"
#include "Scene.h"

LRESULT CALLBACK sr::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_KEYDOWN:
        {
            Input::KeyCallBack(wParam, (int)INPUT_STATE::PRESSED);
        }
        break;
        case WM_KEYUP:
        {
            Input::KeyCallBack(wParam, (int)INPUT_STATE::UP);
        }
        break;
        case WM_LBUTTONDBLCLK:
        {
            Input::KeyCallBack(0x01, (int)INPUT_STATE::DOUBLECLICK);
        }
        break;
        case WM_LBUTTONUP:
        {
            Input::KeyCallBack(0x01, (int)INPUT_STATE::UP);
        }
        break;
        case WM_LBUTTONDOWN:
        {
            Input::KeyCallBack(0x01, (int)INPUT_STATE::PRESSED);
        }
        break;
        case WM_RBUTTONDBLCLK:
        {
            Input::KeyCallBack(0x02, (int)INPUT_STATE::DOUBLECLICK);
        }
        break;
        case WM_RBUTTONUP:
        {
            Input::KeyCallBack(0x02, (int)INPUT_STATE::UP);
        }
        break;
        case WM_RBUTTONDOWN:
        {
            Input::KeyCallBack(0x02, (int)INPUT_STATE::PRESSED);
        }
        break;
        case WM_MBUTTONDBLCLK:
        {
            Input::KeyCallBack(0x04, (int)INPUT_STATE::DOUBLECLICK);
        }
        break;
        case WM_MBUTTONUP:
        {
            Input::KeyCallBack(0x04, (int)INPUT_STATE::UP);
        }
        break;
        case WM_MBUTTONDOWN:
        {
            Input::KeyCallBack(0x04, (int)INPUT_STATE::PRESSED);
        }
        break;
        case WM_CLOSE:
            {
                DestroyWindow(hWnd);
            }
            break;
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            }
        case WM_SIZE:
            {
                RECT rect;
                GetClientRect(hWnd, &rect);
                glm::ivec2 size = { rect.right - rect.left, rect.bottom - rect.top };
                sr::Window::GetInstance().Resize(size);
            }
            break;
        default:
            {
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }
            break;
    }
}

void sr::Window::Init(std::wstring title, glm::ivec2 size)
{
    s_Title = title;
    s_Size = size;
    s_HInstance = GetModuleHandle(nullptr);

    const wchar_t* className = L"WINDOWCLASS";

    WNDCLASS wndClass = {};
    wndClass.lpszClassName = className;
    wndClass.hInstance = s_HInstance;
    wndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.style = CS_DBLCLKS;
    wndClass.lpfnWndProc = WindowProc;
    RegisterClass(&wndClass);

    DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

    RECT rect;
    rect.left = 250;
    rect.top = 250;
    rect.right = rect.left + size.x;
    rect.bottom = rect.top + size.y;

    AdjustWindowRect(&rect, style, false);

    s_HWnd = CreateWindowEx(
        0,
        className,
        s_Title.c_str(),
        style,
        rect.left,
        rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        s_HInstance,
        NULL
    );

    ShowWindow(s_HWnd, SW_SHOW);
}

sr::Window::~Window()
{
    const wchar_t* className = L"WINDOWCLASS";
    UnregisterClass(className, s_HInstance);
}

bool sr::Window::ProcessMessages()
{
    Input::ResetInput();
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE))
    {
        if (msg.message == WM_QUIT) return false;
       
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

sr::Window& sr::Window::GetInstance()
{
    static Window window;
    return window;
}

void sr::Window::Resize(glm::ivec2 size)
{
    if (s_BitMapMemory)
    {
        VirtualFree(s_BitMapMemory, 0, MEM_RELEASE);
        delete[] s_DepthBuffer;
    }

    s_Size = size;
    s_BitMapSize = size;

    s_BitMapInfo.bmiHeader.biSize = sizeof(s_BitMapInfo.bmiHeader);
    s_BitMapInfo.bmiHeader.biWidth = size.x;
    s_BitMapInfo.bmiHeader.biHeight = size.y;
    s_BitMapInfo.bmiHeader.biPlanes = 1;
    s_BitMapInfo.bmiHeader.biBitCount = 32;
    s_BitMapInfo.bmiHeader.biCompression = BI_RGB;

    int m_BitMapMemorySize = (s_BitMapSize.x * s_BitMapSize.y) * 4;
    s_BitMapMemory = VirtualAlloc(0, m_BitMapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    s_DepthBuffer = new float[s_BitMapSize.x * s_BitMapSize.y];

    //sr::Scene::GetInstance().camera.RecalculateProjectionMatrix(); due to call this in Camera::Move() every frame
}

void sr::Window::Update()
{
    HDC DeviceContext = GetDC(s_HWnd);
    RECT rect = {};
    GetClientRect(s_HWnd, &rect);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;
    StretchDIBits(DeviceContext,
        0, 0, s_BitMapSize.x, s_BitMapSize.y,
        0, 0, windowWidth, windowHeight,
        s_BitMapMemory,
        &s_BitMapInfo,
        DIB_RGB_COLORS, SRCCOPY
    );
    ReleaseDC(s_HWnd, DeviceContext);
}

void sr::Window::Clear(glm::ivec3 color)
{
    for (size_t i = 0; i < s_BitMapSize.x; i++)
    {
        for (size_t j = 0; j < s_BitMapSize.y; j++)
        {
            Renderer::FillPixel(glm::ivec2(i, j), color);
            s_DepthBuffer[j * s_BitMapSize.x + i] = 1.0f;
        }
    }
}