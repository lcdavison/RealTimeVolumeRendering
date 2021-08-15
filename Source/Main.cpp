#include <Windows.h>
#include <stdexcept>
#include <memory>

#include "ErrorHandling.h"
#include "OpenGLContext.h"

static HWND WindowHandle = {};
static std::unique_ptr<RTVR::OpenGL::Context> OpenGLContext = {};
static BOOL IsProgramRunning = { FALSE };

constexpr UINT WindowWidth = { 1920 };
constexpr UINT WindowHeight = { 1080 };

LRESULT CALLBACK WindowEventHandler(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    switch (Message)
    {
        case WM_DESTROY:
            {
                ::IsProgramRunning = FALSE;
            }
            break;
    }

    return DefWindowProc(Window, Message, WParam, LParam);
}

VOID CreateAndRegisterWindowClass(HINSTANCE Instance)
{
    WNDCLASSEX WindowClass = {};

    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.lpszClassName = L"RealTimeVolumeRenderingWindow";
    WindowClass.hInstance = Instance;
    WindowClass.hIcon = ::LoadIcon(nullptr, IDI_APPLICATION);
    WindowClass.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    WindowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    WindowClass.style = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.cbClsExtra = 0;
    WindowClass.lpszMenuName = L"RealTimeVolumeRenderingWindow";
    WindowClass.lpfnWndProc = &::WindowEventHandler;

    if (!::RegisterClassEx(&WindowClass))
    {
        throw std::runtime_error("Failed to register window class");
    }
}

POINT CalculateCenteredWindowPosition()
{
    INT ScreenWidth = { ::GetSystemMetrics(SM_CXSCREEN) };
    INT ScreenHeight = { ::GetSystemMetrics(SM_CYSCREEN) };

    POINT CenteredWindowPosition = {};

    CenteredWindowPosition.x = (ScreenWidth - WindowWidth) / 2;
    CenteredWindowPosition.y = (ScreenHeight - WindowHeight) / 2;

    return CenteredWindowPosition;
}

VOID CreateApplicationWindow(HINSTANCE Instance)
{
    ::CreateAndRegisterWindowClass(Instance);

    POINT CenteredWindowPosition = { ::CalculateCenteredWindowPosition() };

    WindowHandle = ::CreateWindowEx(0, 
                                    L"RealTimeVolumeRenderingWindow", L"Real Time Volume Rendering", 
                                    WS_OVERLAPPEDWINDOW, 
                                    CenteredWindowPosition.x, CenteredWindowPosition.y,
                                    WindowWidth, WindowHeight, 
                                    nullptr, nullptr, 
                                    Instance, 
                                    nullptr);

    if (!WindowHandle)
    {
        throw std::runtime_error("Failed to create window");
    }

    ::ShowWindow(WindowHandle, TRUE);

    ::UpdateWindow(WindowHandle);
}

VOID HandleEvents()
{
    MSG Message = {};

    if (::PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
    {
        ::TranslateMessage(&Message);

        ::DispatchMessage(&Message);
    }
}

VOID Render()
{
    OpenGLContext->ClearBackBuffer();

    OpenGLContext->SwapBuffers();
}

VOID RunProgram(HINSTANCE Instance)
{
    ::CreateApplicationWindow(Instance);

    OpenGLContext = std::make_unique<RTVR::OpenGL::Context>(WindowHandle);

    ::IsProgramRunning = TRUE;

    while (::IsProgramRunning)
    {
        ::HandleEvents();

        ::Render();
    }
}

INT WINAPI WinMain(HINSTANCE Instance, HINSTANCE, LPSTR CommandLine, INT ShowCommand)
{
    RTVR::ErrorHandling::ExecuteAndCatchException(::RunProgram, Instance);

    return 0;
}