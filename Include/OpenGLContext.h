#pragma once

#include <Windows.h>

#include <memory>

namespace RTVR::OpenGL
{
    class Context
    {
        class Implementation;
        std::unique_ptr<Implementation> Impl_;

    public:
        Context(HWND WindowHandle);

        Context(const Context& OtherContext) = delete;
        Context& operator = (const Context& OtherContext) = delete;

        ~Context();

        VOID ClearBackBuffer();
        VOID SwapBuffers();
    };
}