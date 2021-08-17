#pragma once

#include <Windows.h>

#include <memory>
#include <string>

namespace RTVR::OpenGL
{
    #pragma region Enums
    enum class PixelFormat
    {
        GrayScale,
        RGB, 
        RGBA
    };

    enum class PixelDataType
    {
        UnsignedByte,
        Byte,
        UnsignedInt,
        Int,
        Float
    };

    enum class ShaderType
    {
        Vertex,
        Pixel
    };
    #pragma endregion Enums
    
    class Context
    {
        class Implementation;
        std::unique_ptr<Implementation> Impl_;

    public:
        Context(HWND WindowHandle);

        Context(const Context& OtherContext) = delete;
        Context& operator = (const Context& OtherContext) = delete;

        ~Context();

        UINT CreateBuffer(std::size_t BufferSizeInBytes, VOID* InitialBufferData = nullptr);
        VOID DeleteBuffer(UINT OpenGLBufferIndex);

        UINT CreateTexture2D(UINT Width, UINT Height, std::byte* TextureData, PixelFormat TextureFormat = PixelFormat::RGB, PixelDataType DataFormat = PixelDataType::UnsignedInt);
        VOID DeleteTexture2D(UINT OpenGLTextureIndex);
        VOID BindTexture2D(UINT TextureID);

        UINT CreateShader(std::string&& ShaderSourceCode, ShaderType ShaderType);
        VOID DeleteShader(UINT OpenGLShaderIndex);
        VOID ClearBackBuffer();
        VOID SwapBuffers();
    };
}