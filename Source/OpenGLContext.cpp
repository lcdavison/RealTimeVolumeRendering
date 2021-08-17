#include "OpenGLContext.h"

#include <stdexcept>
#include <string>

#include "OpenGLFunction.h"
#include <gl/GL.h>
#include <OpenGL/glext.h>

#define CREATE_OPENGL_FUNCTION_WRAPPER(FunctionName, FunctionPointer) RTVR::OpenGL::OpenGLFunctionWrapper<FunctionPointer> FunctionName { #FunctionName }

namespace RTVR::OpenGL
{
    #pragma region Context_OpenGL_Functions

    CREATE_OPENGL_FUNCTION_WRAPPER(glGetStringi, PFNGLGETSTRINGIPROC);

    CREATE_OPENGL_FUNCTION_WRAPPER(glCreateVertexArrays, PFNGLCREATEVERTEXARRAYSPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glBindVertexArray, PFNGLBINDVERTEXARRAYPROC);

    CREATE_OPENGL_FUNCTION_WRAPPER(glVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glDisableVertexAttribArray, PFNGLDISABLEVERTEXATTRIBARRAYPROC);

    CREATE_OPENGL_FUNCTION_WRAPPER(glCreateBuffers, PFNGLCREATEBUFFERSPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glDeleteBuffers, PFNGLDELETEBUFFERSPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glBindBuffer, PFNGLBINDBUFFERPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glBufferData, PFNGLBUFFERDATAPROC);

    CREATE_OPENGL_FUNCTION_WRAPPER(glCreateTextures, PFNGLCREATETEXTURESPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glTextureParameteri, PFNGLTEXTUREPARAMETERIPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glTextureParameterf, PFNGLTEXTUREPARAMETERFPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glBindTextures, PFNGLBINDTEXTURESPROC);

    CREATE_OPENGL_FUNCTION_WRAPPER(glGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glUniform3fv, PFNGLUNIFORM3FVPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glUniform2fv, PFNGLUNIFORM2FVPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glUniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC);

    CREATE_OPENGL_FUNCTION_WRAPPER(glCreateShader, PFNGLCREATESHADERPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glDeleteShader, PFNGLDELETESHADERPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glAttachShader, PFNGLATTACHSHADERPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glDetachShader, PFNGLDETACHSHADERPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glShaderSource, PFNGLSHADERSOURCEPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glCompileShader, PFNGLCOMPILESHADERPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glGetShaderiv, PFNGLGETSHADERIVPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC);

    CREATE_OPENGL_FUNCTION_WRAPPER(glCreateProgram, PFNGLCREATEPROGRAMPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glDeleteProgram, PFNGLDELETEPROGRAMPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glLinkProgram, PFNGLLINKPROGRAMPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glValidateProgram, PFNGLVALIDATEPROGRAMPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glGetProgramiv, PFNGLGETPROGRAMIVPROC);

    CREATE_OPENGL_FUNCTION_WRAPPER(glUseProgram, PFNGLUSEPROGRAMPROC);

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

        VOID ThrowExceptionOnCompilationFailure(UINT OpenGLShaderIndex);

        GLenum ShaderTypeToOpenGLShaderType(ShaderType ShaderType);
    private:
        VOID SetupPixelFormat();

    };

    VOID Context::Implementation::CreateContext(HWND WindowHandle)
    {
        DeviceContext_ = { ::GetDC(WindowHandle) };

        SetupPixelFormat();

        OpenGLContext_ = ::wglCreateContext(DeviceContext_);
        ::wglMakeCurrent(DeviceContext_, OpenGLContext_);
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

    VOID Context::Implementation::ThrowExceptionOnCompilationFailure(UINT OpenGLShaderIndex)
    {
        GLint HasShaderCompiledSuccessfully = {};
        glGetShaderiv(OpenGLShaderIndex, GL_COMPILE_STATUS, &HasShaderCompiledSuccessfully);

        if (HasShaderCompiledSuccessfully == GL_FALSE)
        {
            GLint ShaderLogLength = {};
            glGetShaderiv(OpenGLShaderIndex, GL_INFO_LOG_LENGTH, &ShaderLogLength);

            std::string ShaderLog = {};
            ShaderLog.resize(ShaderLogLength);

            glGetShaderInfoLog(OpenGLShaderIndex, ShaderLogLength, nullptr, ShaderLog.data());

            glDeleteShader(OpenGLShaderIndex);

            throw std::runtime_error("Shader Compilation Error\n" + ShaderLog);
        }
    }
    GLenum Context::Implementation::ShaderTypeToOpenGLShaderType(ShaderType ShaderType)
    {
        switch (ShaderType)
        {
        case ShaderType::Vertex:
            return GL_VERTEX_SHADER;
        case ShaderType::Pixel:
            return GL_FRAGMENT_SHADER;
        default:
            throw std::runtime_error("ShaderTypeToOpenGLShaderType: Unsupported shader type");
        }
    }
    #pragma endregion Context_Private_Implementation

    Context::Context(HWND WindowHandle)
        : Impl_ { new Context::Implementation{} }
    {
        Impl_->CreateContext(WindowHandle);

        OpenGLFunctionLoader::LoadFunctions();
    }

    Context::~Context()
    {
        Impl_->DeleteContext();
    }

    UINT Context::CreateShader(std::string&& ShaderSourceCode, ShaderType ShaderType)
    {
        GLenum OpenGLShaderType = { Impl_->ShaderTypeToOpenGLShaderType(ShaderType) };

        GLuint NewShaderIndex = { glCreateShader(OpenGLShaderType) };

        std::string SourceCode = { ShaderSourceCode };
        GLchar* SourceCodeAddress = { ShaderSourceCode.data() };
        GLint ShaderSourceCodeLength = { static_cast<GLint>(ShaderSourceCode.length()) };

        glShaderSource(NewShaderIndex, 1, &SourceCodeAddress, &ShaderSourceCodeLength);

        glCompileShader(NewShaderIndex);

        Impl_->ThrowExceptionOnCompilationFailure(NewShaderIndex);

        return NewShaderIndex;
    }

    VOID Context::DeleteShader(UINT OpenGLShaderIndex)
    {
        glDeleteShader(OpenGLShaderIndex);
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