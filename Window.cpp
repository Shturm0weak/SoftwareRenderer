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
            Input::KeyCallBack(wParam, (int)INPUTSTATE::PRESSED);
        }
        break;
        case WM_KEYUP:
        {
            Input::KeyCallBack(wParam, (int)INPUTSTATE::UP);
        }
        break;
        case WM_LBUTTONDBLCLK:
        {
            Input::KeyCallBack(0x01, (int)INPUTSTATE::DOUBLECLICK);
        }
        break;
        case WM_LBUTTONUP:
        {
            Input::KeyCallBack(0x01, (int)INPUTSTATE::UP);
        }
        break;
        case WM_LBUTTONDOWN:
        {
            Input::KeyCallBack(0x01, (int)INPUTSTATE::PRESSED);
        }
        break;
        case WM_RBUTTONDBLCLK:
        {
            Input::KeyCallBack(0x02, (int)INPUTSTATE::DOUBLECLICK);
        }
        break;
        case WM_RBUTTONUP:
        {
            Input::KeyCallBack(0x02, (int)INPUTSTATE::UP);
        }
        break;
        case WM_RBUTTONDOWN:
        {
            Input::KeyCallBack(0x02, (int)INPUTSTATE::PRESSED);
        }
        break;
        case WM_MBUTTONDBLCLK:
        {
            Input::KeyCallBack(0x04, (int)INPUTSTATE::DOUBLECLICK);
        }
        break;
        case WM_MBUTTONUP:
        {
            Input::KeyCallBack(0x04, (int)INPUTSTATE::UP);
        }
        break;
        case WM_MBUTTONDOWN:
        {
            Input::KeyCallBack(0x04, (int)INPUTSTATE::PRESSED);
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
                Window& window = sr::Window::GetInstance();
                window.Resize(window.GetSize(hWnd));
            }
            break;
        default:
            {
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }
            break;
    }
}

void sr::Window::Init(const std::wstring& title, const glm::ivec2& size, const glm::vec2& screenPixelsInBitMapPixels)
{
    m_Title = title;
    m_HInstance = GetModuleHandle(nullptr);
    m_ScreenPixelsInBitMapPixels = screenPixelsInBitMapPixels;
    const wchar_t* className = L"WINDOWCLASS";

    WNDCLASS wndClass = {};
    wndClass.lpszClassName = className;
    wndClass.hInstance = m_HInstance;
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

    m_HWnd = CreateWindowEx(
        0,
        className,
        m_Title.c_str(),
        style,
        rect.left,
        rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        m_HInstance,
        NULL
    );

    ShowWindow(m_HWnd, SW_SHOW);

    m_BlendFunction.BlendOp = AC_SRC_OVER;
    m_BlendFunction.BlendFlags = 0;
    m_BlendFunction.SourceConstantAlpha = 0xff;
    m_BlendFunction.AlphaFormat = AC_SRC_ALPHA;

}

sr::Window::~Window()
{
    const wchar_t* className = L"WINDOWCLASS";
    UnregisterClass(className, m_HInstance);
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

inline sr::Window& sr::Window::GetInstance()
{
    static Window window;
    return window;
}

glm::ivec2 sr::Window::GetSize(HWND hWnd)
{
    RECT rect;
    GetClientRect(hWnd, &rect);
    return { rect.right - rect.left, rect.bottom - rect.top };
}

void sr::Window::Resize(const glm::ivec2& size)
{
    if (m_BitMapMemory)
    {
        //DeleteObject(m_HBitMap);
        VirtualFree(m_BitMapMemory, 0, MEM_RELEASE);
        delete[] m_DepthBuffer;
    }

    m_BitMapSize = (glm::vec2)size / m_ScreenPixelsInBitMapPixels;

    m_BitMapInfo.bmiHeader.biSize = sizeof(m_BitMapInfo.bmiHeader);
    m_BitMapInfo.bmiHeader.biWidth = m_BitMapSize.x;
    m_BitMapInfo.bmiHeader.biHeight = m_BitMapSize.y;
    m_BitMapInfo.bmiHeader.biPlanes = 1;
    m_BitMapInfo.bmiHeader.biBitCount = 32;
    m_BitMapInfo.bmiHeader.biCompression = BI_RGB;
    m_BitMapInfo.bmiHeader.biSizeImage = m_BitMapSize.x * m_BitMapSize.y * 4;

    int m_BitMapMemorySize = m_BitMapSize.x * m_BitMapSize.y * 4;
    m_BitMapMemory = VirtualAlloc(0, m_BitMapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    m_DepthBuffer = new float[m_BitMapSize.x * m_BitMapSize.y];

    //m_HBitMap = CreateDIBSection(CreateCompatibleDC(GetDC(m_HWnd)), &m_BitMapInfo, DIB_RGB_COLORS, &m_BitMapMemory, NULL, 0x0);
    //sr::Scene::GetInstance().camera.RecalculateProjectionMatrix(); due to call this in Camera::Move() every frame
}

void sr::Window::SetTitle(const std::wstring& title)
{
    m_Title = title;
    SetWindowTextW(m_HWnd, m_Title.c_str());
}

void sr::Window::Update()
{
    HDC DeviceContext = GetDC(m_HWnd);
    RECT rect = {};
    GetClientRect(m_HWnd, &rect);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;
    //SetDIBitsToDevice(DeviceContext, 0, 0, windowWidth, windowHeight, 0, 0, 0, m_BitMapSize.y, m_BitMapMemory, &m_BitMapInfo, DIB_RGB_COLORS);
    SetStretchBltMode(DeviceContext, BLACKONWHITE);
    StretchDIBits(
        DeviceContext,
        0, 0, windowWidth, windowHeight,
        0, 0, m_BitMapSize.x, m_BitMapSize.y,
        m_BitMapMemory,
        &m_BitMapInfo,
        DIB_RGB_COLORS, SRCCOPY
    );
    ReleaseDC(m_HWnd, DeviceContext);
    
    //SelectObject(DeviceContext, m_HBitMap);
    //AlphaBlend(CreateCompatibleDC(GetDC(m_HWnd)),
    //    0, 0,
    //    windowWidth,
    //    windowHeight,
    //    DeviceContext,
    //    0, 0, m_BitMapSize.x, m_BitMapSize.y, 
    //    m_BlendFunction
    //);
}

void sr::Window::Clear(const glm::ivec4& color)
{
    auto ClearBitMapJob = [](const glm::ivec2& bitMapSize, const glm::ivec4& color, float* depthBuffer, size_t start, size_t end)
    {
        for (size_t i = start; i < end; i++)
        {
            for (size_t j = 0; j < bitMapSize.y; j++)
            {
                Renderer::FillPixel(glm::ivec2(i, j), color);
                depthBuffer[j * bitMapSize.x + i] = 1.0f;
            }
        }
    };

    size_t numThreads = ThreadPool::GetInstance().GetAmountOfThreads();
    float dif = (float)m_BitMapSize.x / (float)numThreads;

    for (size_t k = 0; k < numThreads - 1; k++)
    {
        m_SyncParams.s_Ready[k] = false;
        ThreadPool::GetInstance().Enqueue([=] {
            ClearBitMapJob(m_BitMapSize, color, m_DepthBuffer, k * dif, k * dif + dif);
            m_SyncParams.ThreadFinished(k);
        });
    }

    m_SyncParams.s_Ready[numThreads - 1] = false;
    ThreadPool::GetInstance().Enqueue([=] {
        ClearBitMapJob(m_BitMapSize, color, m_DepthBuffer, (numThreads - 1) * dif, m_BitMapSize.x);
        m_SyncParams.ThreadFinished(numThreads - 1);
    });

    m_SyncParams.WaitForAllThreads();
}