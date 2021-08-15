#include "OpenGLContext.h"

#include <stdexcept>
#include <string>

#include "OpenGLFunction.h"
#include <gl/GL.h>
#include <OpenGL/glext.h>

#define CREATE_OPENGL_FUNCTION_WRAPPER(FunctionName, FunctionPointer) RTVR::OpenGL::OpenGLFunctionWrapper<FunctionPointer> FunctionName { #FunctionName };

namespace RTVR::OpenGL
{
    #pragma region Context_OpenGL_Functions
    
    CREATE_OPENGL_FUNCTION_WRAPPER(glGetStringi, PFNGLGETSTRINGIPROC)

    CREATE_OPENGL_FUNCTION_WRAPPER(glCreateVertexArrays, PFNGLCREATEVERTEXARRAYSPROC)
    CREATE_OPENGL_FUNCTION_WRAPPER(glDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC)
    CREATE_OPENGL_FUNCTION_WRAPPER(glBindVertexArray, PFNGLBINDVERTEXARRAYPROC)

    CREATE_OPENGL_FUNCTION_WRAPPER(glCreateBuffers, PFNGLCREATEBUFFERSPROC)
    CREATE_OPENGL_FUNCTION_WRAPPER(glDeleteBuffers, PFNGLDELETEBUFFERSPROC)

    CREATE_OPENGL_FUNCTION_WRAPPER(glCreateTextures, PFNGLCREATETEXTURESPROC)
    CREATE_OPENGL_FUNCTION_WRAPPER(glTextureParameteri, PFNGLTEXTUREPARAMETERIPROC)
    CREATE_OPENGL_FUNCTION_WRAPPER(glTextureParameterf, PFNGLTEXTUREPARAMETERFPROC)
    CREATE_OPENGL_FUNCTION_WRAPPER(glBindTextures, PFNGLBINDTEXTURESPROC)

    CREATE_OPENGL_FUNCTION_WRAPPER(glCreateShader, PFNGLCREATESHADERPROC)
    CREATE_OPENGL_FUNCTION_WRAPPER(glDeleteShader, PFNGLDELETESHADERPROC)
    CREATE_OPENGL_FUNCTION_WRAPPER(glAttachShader, PFNGLATTACHSHADERPROC)
    CREATE_OPENGL_FUNCTION_WRAPPER(glDetachShader, PFNGLDETACHSHADERPROC)
    CREATE_OPENGL_FUNCTION_WRAPPER(glCompileShader, PFNGLCOMPILESHADERPROC)

    CREATE_OPENGL_FUNCTION_WRAPPER(glCreateProgram, PFNGLCREATEPROGRAMPROC)
    CREATE_OPENGL_FUNCTION_WRAPPER(glDeleteProgram, PFNGLDELETEPROGRAMPROC)
    CREATE_OPENGL_FUNCTION_WRAPPER(glValidateProgram, PFNGLVALIDATEPROGRAMPROC)

    #pragma endregion Context_OpenGL_Functions

    #pragma region Context_Private_Implementation
    class Context::Implementation
    {
        HDC DeviceContext_;
        HGLRC OpenGLContext_;

    public:
        VOID CreateContext(HWND WindowHandle);
        VOID DeleteContext();

        VOID SwapBuffers();

        VOID CheckForValidContext();

    private:
        VOID SetupPixelFormat();

    };

    VOID Context::Implementation::CreateContext(HWND WindowHandle)
    {
        DeviceContext_ = { ::GetDC(WindowHandle) };

        SetupPixelFormat();

        OpenGLContext_ = ::wglCreateContext(DeviceContext_);
        ::wglMakeCurrent(DeviceContext_, OpenGLContext_);

        OpenGLFunctionLoader::LoadFunctions();
    }

    VOID Context::Implementation::SetupPixelFormat()
    {
        PIXELFORMATDESCRIPTOR PixelFormatDescriptor = {};

        PixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        PixelFormatDescriptor.nVersion = 1;
        PixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        PixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
        PixelFormatDescriptor.cColorBits = 32;
        PixelFormatDescriptor.cDepthBits = 24;
        PixelFormatDescriptor.cStencilBits = 8;
        PixelFormatDescriptor.cAccumBits = 0;

        INT PixelFormatIndex = { ::ChoosePixelFormat(DeviceContext_, &PixelFormatDescriptor) };

        if (PixelFormatIndex == 0)
        {
            throw std::runtime_error("Failed to choose pixel format");
        }

        BOOL IsSetPixelFormatSuccessful = { ::SetPixelFormat(DeviceContext_, PixelFormatIndex, &PixelFormatDescriptor) };

        if (!IsSetPixelFormatSuccessful)
        {
            throw std::runtime_error("Failed to set pixel format");
        }
    }

    VOID Context::Implementation::DeleteContext()
    {
        ::wglDeleteContext(OpenGLContext_);
    }

    VOID Context::Implementation::SwapBuffers()
    {
        ::SwapBuffers(DeviceContext_);
    }

    VOID Context::Implementation::CheckForValidContext()
    {
        if (!OpenGLContext_)
        {
            throw std::runtime_error("A valid context has not been created");
        }
    }
    #pragma endregion Context_Private_Implementation

    Context::Context(HWND WindowHandle)
        : Impl_ { new Context::Implementation{} }
    {
        Impl_->CreateContext(WindowHandle);
    }

    Context::~Context()
    {
        Impl_->DeleteContext();
    }

    VOID Context::ClearBackBuffer()
    {
        Impl_->CheckForValidContext();

        glClearColor(0.0f, 0.5f, 0.5f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    VOID Context::SwapBuffers()
    {
        Impl_->CheckForValidContext();

        Impl_->SwapBuffers();
    }
}