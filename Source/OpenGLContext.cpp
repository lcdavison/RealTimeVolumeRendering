#include "OpenGLContext.h"

#include <stdexcept>
#include <string>
#include <sstream>

#include <DirectXMath.h>

#include "OpenGLFunction.h"
#include <gl/GL.h>
#include <OpenGL/glext.h>
#include <OpenGL/glcorearb.h>

#define CREATE_OPENGL_FUNCTION_WRAPPER(FunctionName, FunctionPointer) RTVR::OpenGL::OpenGLFunctionWrapper<FunctionPointer> FunctionName { #FunctionName }

using namespace DirectX;

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
    CREATE_OPENGL_FUNCTION_WRAPPER(glNamedBufferSubData, PFNGLNAMEDBUFFERSUBDATAPROC);

    CREATE_OPENGL_FUNCTION_WRAPPER(glCreateTextures, PFNGLCREATETEXTURESPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glTextureParameteri, PFNGLTEXTUREPARAMETERIPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glTextureParameterf, PFNGLTEXTUREPARAMETERFPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glTextureParameterfv, PFNGLTEXTUREPARAMETERFVPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glBindTextures, PFNGLBINDTEXTURESPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glActiveTexture, PFNGLACTIVETEXTUREPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glBindTextureUnit, PFNGLBINDTEXTUREUNITPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glTextureStorage2D, PFNGLTEXTURESTORAGE2DPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glTexStorage2D, PFNGLTEXSTORAGE2DPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glTextureSubImage2D, PFNGLTEXTURESUBIMAGE2DPROC);

    CREATE_OPENGL_FUNCTION_WRAPPER(glGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC);
    CREATE_OPENGL_FUNCTION_WRAPPER(glUniform1i, PFNGLUNIFORM1IPROC);
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
    CREATE_OPENGL_FUNCTION_WRAPPER(glGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC);

    CREATE_OPENGL_FUNCTION_WRAPPER(glUseProgram, PFNGLUSEPROGRAMPROC);

    CREATE_OPENGL_FUNCTION_WRAPPER(glDebugMessageCallback, PFNGLDEBUGMESSAGECALLBACKPROC);

    #pragma endregion Context_OpenGL_Functions

    #pragma region Context_Private_Implementation
    class Context::Implementation
    {
        HDC DeviceContext_;
        HGLRC OpenGLContext_;

    public:
        using OpenGLTexelInfo = std::pair<GLenum, GLenum>;

        VOID CreateContext(HWND WindowHandle);
        VOID DeleteContext();

        VOID SwapBuffers();

        VOID CheckForValidContext();

        VOID ThrowExceptionOnCompilationFailure(UINT OpenGLShaderID);
        VOID ThrowExceptionOnLinkFailure(UINT OpenGLShaderProgramID);

        OpenGLTexelInfo GetOpenGLTexelLayout(TexelFormat TexelFormat, DataType DataType);
        GLenum GetOpenGLBufferType(BufferType BufferType);
        GLenum GetOpenGLBlendFactor(BlendFactor BlendFactor);
        GLenum GetOpenGLDataType(DataType DataFormat);
        GLenum GetOpenGLShaderType(ShaderType ShaderType);

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

    VOID Context::Implementation::ThrowExceptionOnCompilationFailure(UINT OpenGLShaderID)
    {
        GLint HasShaderCompiledSuccessfully = {};
        glGetShaderiv(OpenGLShaderID, GL_COMPILE_STATUS, &HasShaderCompiledSuccessfully);

        if (HasShaderCompiledSuccessfully == GL_FALSE)
        {
            GLint ShaderLogLength = {};
            glGetShaderiv(OpenGLShaderID, GL_INFO_LOG_LENGTH, &ShaderLogLength);

            std::string ShaderLog = {};
            ShaderLog.resize(ShaderLogLength);

            glGetShaderInfoLog(OpenGLShaderID, ShaderLogLength, nullptr, ShaderLog.data());

            glDeleteShader(OpenGLShaderID);

            throw std::runtime_error("Shader Compilation Error\n" + ShaderLog);
        }
    }

    VOID Context::Implementation::ThrowExceptionOnLinkFailure(UINT OpenGLShaderProgramID)
    {
        GLint HasProgramLinkedSuccessfully = {};
        glGetProgramiv(OpenGLShaderProgramID, GL_LINK_STATUS, &HasProgramLinkedSuccessfully);

        if (HasProgramLinkedSuccessfully == GL_FALSE)
        {
            GLint ProgramInfoLogLength = {};
            glGetProgramiv(OpenGLShaderProgramID, GL_INFO_LOG_LENGTH, &ProgramInfoLogLength);

            std::string ProgramInfoLog = {};
            ProgramInfoLog.resize(ProgramInfoLogLength);

            glGetProgramInfoLog(OpenGLShaderProgramID, ProgramInfoLogLength, nullptr, ProgramInfoLog.data());

            glDeleteProgram(OpenGLShaderProgramID);

            throw std::runtime_error("Failed to link shader programs:\n" + ProgramInfoLog);
        }
    }

    Context::Implementation::OpenGLTexelInfo Context::Implementation::GetOpenGLTexelLayout(TexelFormat TexelFormat, DataType DataType)
    {
        switch (TexelFormat)
        {
        case TexelFormat::GrayScale:
            {
                switch (DataType)
                {
                case DataType::UnsignedByte:
                    {
                        return std::make_pair(GL_RED_INTEGER, GL_R8UI);
                    }
                case DataType::Byte:
                    {
                        return std::make_pair(GL_RED_INTEGER, GL_R8I);
                    }
                case DataType::Float:
                    {
                        return std::make_pair(GL_RED, GL_R32F);
                    }
                }
            }
            break;
        case TexelFormat::RGB:
            {
                switch (DataType)
                {
                case DataType::UnsignedByte:
                    {
                        return std::make_pair(GL_RGB, GL_RGB8UI);
                    }
                case DataType::Byte:
                    {
                        return std::make_pair(GL_RGB, GL_RGB8I);
                    }
                }
            }
            break;
        case TexelFormat::RGBA:
            {
                switch (DataType)
                {
                case DataType::Float:
                    {
                        return std::make_pair(GL_RGBA, GL_RGBA32F);
                    }
                }
            }
            break;
        default:
            throw std::runtime_error("GetOpenGLTexelInformation: Unsupported Texel Format");
        }
    }

    GLenum Context::Implementation::GetOpenGLBufferType(BufferType BufferType)
    {
        switch (BufferType)
        {
        case BufferType::Static:
            return GL_STATIC_DRAW;
        case BufferType::Dynamic:
            return GL_DYNAMIC_DRAW;
        case BufferType::Stream:
            return GL_STREAM_DRAW;
        }
    }

    GLenum Context::Implementation::GetOpenGLBlendFactor(BlendFactor BlendFactor)
    {
        switch (BlendFactor)
        {
        case BlendFactor::One:
            return GL_ONE;
        case BlendFactor::OneMinusSourceAlpha:
            return GL_ONE_MINUS_SRC_ALPHA;
        }
    }

    GLenum Context::Implementation::GetOpenGLDataType(DataType DataType)
    {
        switch (DataType)
        {
        case DataType::UnsignedByte:
            return GL_UNSIGNED_BYTE;
        case DataType::Byte:
            return GL_BYTE;
        case DataType::UnsignedInt:
            return GL_UNSIGNED_INT;
        case DataType::Int:
            return GL_INT;
        case DataType::Float:
            return GL_FLOAT;
        default:
            throw std::runtime_error("GetOpenGLDataType: Unsupported pixel data type");
        }
    }

    GLenum Context::Implementation::GetOpenGLShaderType(ShaderType ShaderType)
    {
        switch (ShaderType)
        {
        case ShaderType::Vertex:
            return GL_VERTEX_SHADER;
        case ShaderType::Pixel:
            return GL_FRAGMENT_SHADER;
        default:
            throw std::runtime_error("GetOpenGLShaderType: Unsupported shader type");
        }
    }
    #pragma endregion Context_Private_Implementation

    Context::Context(HWND WindowHandle)
        : Impl_ { new Context::Implementation{} }
    {
        Impl_->CreateContext(WindowHandle);

        OpenGLFunctionLoader::LoadFunctions();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
    }

    Context::~Context()
    {
        Impl_->DeleteContext();
    }

    UINT Context::CreateVertexArray()
    {
        GLuint NewVertexArrayID = {};
        glCreateVertexArrays(1, &NewVertexArrayID);

        if (NewVertexArrayID == 0)
        {
            throw std::runtime_error("Failed to create OpenGL Vertex Array");
        }

        return NewVertexArrayID;
    }

    VOID Context::DeleteVertexArray(UINT OpenGLVertexArrayID)
    {
        glDeleteVertexArrays(1, &OpenGLVertexArrayID);
    }

    VOID Context::BindVertexArray(UINT OpenGLVertexArrayID)
    {
        glBindVertexArray(OpenGLVertexArrayID);
    }

    VOID Context::CreateVertexAttribute(UINT OpenGLVertexAttributeArrayID, VertexAttribute Attribute, UINT OpenGLAttributeBufferID, UINT AttributeDimensionCount, DataType DataType, SIZE_T ItemSeparationInBytes, SIZE_T OffsetOfFirstItemInBytes, bool IsNormalized)
    {
        glBindVertexArray(OpenGLVertexAttributeArrayID);
        glBindBuffer(GL_ARRAY_BUFFER, OpenGLAttributeBufferID);

        GLenum OpenGLDataType = { Impl_->GetOpenGLDataType(DataType) };
        glVertexAttribPointer(static_cast<GLuint>(Attribute), AttributeDimensionCount, OpenGLDataType, IsNormalized, ItemSeparationInBytes, reinterpret_cast<void*>(OffsetOfFirstItemInBytes));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    VOID Context::EnableVertexAttributes(UINT OpenGLVertexAttributeArrayID, std::vector<VertexAttribute>&& Attributes)
    {
        std::vector<VertexAttribute> VertexAttributes = { Attributes };

        glBindVertexArray(OpenGLVertexAttributeArrayID);

        for (VertexAttribute Attribute : VertexAttributes)
        {
            glEnableVertexAttribArray(static_cast<GLuint>(Attribute));
        }
    }

    VOID Context::DisableVertexAttributes(UINT OpenGLVertexAttributeArrayID, std::vector<VertexAttribute>&& Attributes)
    {
        std::vector<VertexAttribute> VertexAttributes = { Attributes };

        for (VertexAttribute Attribute : VertexAttributes)
        {
            glDisableVertexAttribArray(static_cast<GLuint>(Attribute));
        }

        glBindVertexArray(0);
    }

    UINT Context::CreateBuffer(SIZE_T BufferSizeInBytes, BufferType BufferType, VOID* InitialBufferData)
    {
        GLuint NewBufferID = {};
        glCreateBuffers(1, &NewBufferID);

        if (NewBufferID == 0)
        {
            throw std::runtime_error("Failed to create OpenGL Buffer");
        }

        GLenum OpenGLBufferType = { Impl_->GetOpenGLBufferType(BufferType) };

        glBindBuffer(GL_ARRAY_BUFFER, NewBufferID);
        glBufferData(GL_ARRAY_BUFFER, BufferSizeInBytes, InitialBufferData, OpenGLBufferType);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        return NewBufferID;
    }

    VOID Context::DeleteBuffer(UINT OpenGLBufferID)
    {
        glDeleteBuffers(1, &OpenGLBufferID);
    }

    VOID Context::BindBuffer(UINT OpenGLBufferID)
    {
        glBindBuffer(GL_ARRAY_BUFFER, OpenGLBufferID);
    }

    VOID Context::UpdateBuffer(UINT OpenGLBufferID, VOID* Data, SIZE_T DataSizeInBytes, SIZE_T StartOffsetInBuffer)
    {
        glNamedBufferSubData(OpenGLBufferID, StartOffsetInBuffer, DataSizeInBytes, Data);
    }

    UINT Context::CreateTexture2D(UINT Width, UINT Height, VOID* TextureData, UINT TextureRowAlignment, TexelFormat TexelFormat, DataType DataType)
    {
        GLuint NewTextureID = {};

        glCreateTextures(GL_TEXTURE_2D, 1, &NewTextureID);

        if (NewTextureID == 0)
        {
            throw std::runtime_error("Failed to create OpenGL Texture");
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, TextureRowAlignment);

        auto [OpenGLTexelFormat, OpenGLTexelLayout] = Impl_->GetOpenGLTexelLayout(TexelFormat, DataType);
        GLenum OpenGLDataType = { Impl_->GetOpenGLDataType(DataType) };

        glTextureParameteri(NewTextureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(NewTextureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        GLfloat BorderColour [4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glTextureParameterfv(NewTextureID, GL_TEXTURE_BORDER_COLOR, BorderColour);

        glTextureParameteri(NewTextureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(NewTextureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTextureStorage2D(NewTextureID, 1, OpenGLTexelLayout, Width, Height);

        glTextureSubImage2D(NewTextureID, 0, 0, 0, Width, Height, OpenGLTexelFormat, OpenGLDataType, TextureData);

        return NewTextureID;
    }

    VOID Context::DeleteTexture2D(UINT OpenGLTextureID)
    {
        glDeleteTextures(1, &OpenGLTextureID);
    }

    VOID Context::BindTexture2D(UINT TextureID, UINT TextureUnitIndex)
    {
        glBindTextureUnit(TextureUnitIndex, TextureID);
    }

    UINT Context::CreateShader(std::string&& ShaderSourceCode, ShaderType ShaderType)
    {
        GLenum OpenGLShaderType = { Impl_->GetOpenGLShaderType(ShaderType) };

        GLuint NewShaderID = { glCreateShader(OpenGLShaderType) };

        std::string SourceCode = { ShaderSourceCode };
        GLchar* SourceCodeAddress = { ShaderSourceCode.data() };
        GLint ShaderSourceCodeLength = { static_cast<GLint>(ShaderSourceCode.length()) };

        glShaderSource(NewShaderID, 1, &SourceCodeAddress, &ShaderSourceCodeLength);

        glCompileShader(NewShaderID);
        Impl_->ThrowExceptionOnCompilationFailure(NewShaderID);

        return NewShaderID;
    }

    VOID Context::DeleteShader(UINT OpenGLShaderID)
    {
        glDeleteShader(OpenGLShaderID);
    }

    UINT Context::CreateShaderProgram(UINT OpenGLVertexShaderID, UINT OpenGLPixelShaderID)
    {
        GLuint NewProgramID = { glCreateProgram() };

        glAttachShader(NewProgramID, OpenGLVertexShaderID);
        glAttachShader(NewProgramID, OpenGLPixelShaderID);

        glLinkProgram(NewProgramID);
        Impl_->ThrowExceptionOnLinkFailure(NewProgramID);

        glDetachShader(NewProgramID, OpenGLVertexShaderID);
        glDetachShader(NewProgramID, OpenGLPixelShaderID);

        return NewProgramID;
    }

    VOID Context::DeleteShaderProgram(UINT OpenGLShaderProgramID)
    {
        glDeleteProgram(OpenGLShaderProgramID);
    }

    VOID Context::BindShaderProgram(UINT OpenGLShaderProgramID)
    {
        glUseProgram(OpenGLShaderProgramID);
    }

    VOID Context::SetUniformInt(UINT OpenGLShaderProgramID, const std::string& UniformName, INT UniformValue)
    {
        glUseProgram(OpenGLShaderProgramID);

        glUniform1i(glGetUniformLocation(OpenGLShaderProgramID, UniformName.c_str()), UniformValue);
    }

    VOID Context::SetUniformMatrix4x4(UINT OpenGLShaderProgramID, const std::string& UniformName, const FLOAT* UniformValue)
    {
        glUseProgram(OpenGLShaderProgramID);        

        INT UniformLocation = { glGetUniformLocation(OpenGLShaderProgramID, UniformName.c_str()) };
        glUniformMatrix4fv(UniformLocation, 1, false, UniformValue);
    }

    VOID Context::SetBlendFunction(BlendFactor SourceBlendFactor, BlendFactor DestinationBlendFactor)
    {
        GLenum OpenGLSourceBlendFactor = { Impl_->GetOpenGLBlendFactor(SourceBlendFactor) };
        GLenum OpenGLDestinationBlendFactor { Impl_->GetOpenGLBlendFactor(DestinationBlendFactor) };

        glBlendFunc(OpenGLSourceBlendFactor, OpenGLDestinationBlendFactor);
    }

    VOID Context::DrawVertices(UINT VertexCount, UINT VertexBufferOffsetInVertices)
    {
        glDrawArrays(GL_TRIANGLES, VertexBufferOffsetInVertices, VertexCount);
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