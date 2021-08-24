#pragma once

#include <Windows.h>

#include <memory>
#include <string>
#include <vector>

namespace RTVR::OpenGL
{
    #pragma region Enums
    enum class TexelFormat
    {
        GrayScale,
        RGB, 
        RGBA
    };

    enum class DataType
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

    enum class VertexAttribute
    {
        Position,
        TextureCoordinate
    };

    enum class BufferType
    {
        Static,
        Dynamic,
        Stream
    };

    enum class BlendFactor
    {
        One,
        OneMinusSourceAlpha
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

        UINT CreateVertexArray();
        VOID DeleteVertexArray(UINT OpenGLVertexArrayID);
        VOID BindVertexArray(UINT OpenGLVertexArrayID);

        VOID CreateVertexAttribute(UINT OpenGLVertexAttributeArrayID, VertexAttribute Attribute,
                                   UINT OpenGLAttributeBufferID, UINT AttributeDimensionCount, 
                                   DataType DataType, 
                                   SIZE_T ItemSeparationInBytes, SIZE_T OffsetOfFirstItemInBytes, 
                                   bool IsNormalized);

        VOID EnableVertexAttributes(UINT OpenGLVertexAttributeArrayID, std::vector<VertexAttribute>&& Attributes);
        VOID DisableVertexAttributes(UINT OpenGLVertexAttributeArrayID, std::vector<VertexAttribute>&& Attributes);

        UINT CreateBuffer(SIZE_T BufferSizeInBytes, BufferType BufferType, VOID* InitialBufferData = nullptr);
        VOID DeleteBuffer(UINT OpenGLBufferID);
        VOID BindBuffer(UINT OpenGLBufferID);
        VOID UpdateBuffer(UINT OpenGLBufferID, VOID* Data, SIZE_T DataSizeInBytes, SIZE_T StartOffsetInBuffer = 0);

        UINT CreateTexture2D(UINT Width, UINT Height, VOID* TextureData, UINT TextureRowAlignment = 4, TexelFormat TexelFormat = TexelFormat::RGB, DataType DataFormat = DataType::UnsignedInt);
        VOID DeleteTexture2D(UINT OpenGLTextureID);
        VOID BindTexture2D(UINT TextureID, UINT TextureUnitIndex);

        UINT CreateShader(std::string&& ShaderSourceCode, ShaderType ShaderType);
        VOID DeleteShader(UINT OpenGLShaderID);

        UINT CreateShaderProgram(UINT VertexShaderID, UINT PixelShaderID);
        VOID DeleteShaderProgram(UINT OpenGLShaderProgramID);
        VOID BindShaderProgram(UINT OpenGLShaderProgramID);

        VOID SetUniformInt(UINT OpenGLShaderProgramID, const std::string& UniformName, INT UniformValue);
        VOID SetUniformMatrix4x4(UINT OpenGLShaderProgramID, const std::string& UniformName, const FLOAT* UniformValue);

        VOID SetBlendFunction(BlendFactor SourceBlendFactor, BlendFactor DestinationBlendFactor);

        VOID DrawVertices(UINT VertexCount, UINT VertexBufferOffsetInVertices = 0);

        VOID ClearBackBuffer();
        VOID SwapBuffers();
    };
}