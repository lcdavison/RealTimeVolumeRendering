#include "Shader.h"

#include <fstream>
#include <stdexcept>

using namespace DirectX;

namespace RTVR::OpenGL
{
    Shader::Shader(Context* OpenGLContext, const std::string& VertexShaderFilePath, const std::string& PixelShaderFilePath)
    {
        UINT VertexShaderID = { LoadAndCompileShaderFromFile(OpenGLContext, VertexShaderFilePath, ShaderType::Vertex) };
        UINT PixelShaderID = { LoadAndCompileShaderFromFile(OpenGLContext, PixelShaderFilePath, ShaderType::Pixel) };

        OpenGLShaderProgramID_ = OpenGLContext->CreateShaderProgram(VertexShaderID, PixelShaderID);
    }

    VOID Shader::Bind(Context* OpenGLContext)
    {
        OpenGLContext->BindShaderProgram(OpenGLShaderProgramID_);
    }

    VOID Shader::UnBind(Context* OpenGLContext)
    {
        OpenGLContext->BindShaderProgram(0);
    }

    VOID Shader::SetUniformMatrix4x4(Context* OpenGLContext, const std::string& UniformName, const XMFLOAT4X4& Matrix)
    {
        OpenGLContext->SetUniformMatrix4x4(OpenGLShaderProgramID_, UniformName, reinterpret_cast<const FLOAT*>(&Matrix.m[0][0]));
    }

    UINT Shader::LoadAndCompileShaderFromFile(Context* OpenGLContext, const std::filesystem::path& ShaderFilePath, ShaderType ShaderType)
    {
        std::fstream ShaderFileStream = {};
        ShaderFileStream.open(ShaderFilePath, std::fstream::in);

        if (!ShaderFileStream.is_open())
        {
            throw std::runtime_error("Failed to load shader: " + ShaderFilePath.string());
        }

        SIZE_T ShaderFileSizeInBytes = { std::filesystem::file_size(ShaderFilePath) };
        SIZE_T ShaderSourceCodeCharacterCount = { ShaderFileSizeInBytes / sizeof(CHAR) };

        std::string ShaderSourceCode = {};
        ShaderSourceCode.resize(ShaderSourceCodeCharacterCount);

        ShaderFileStream.read(ShaderSourceCode.data(), ShaderSourceCodeCharacterCount);

        UINT OpenGLShaderIndex = { OpenGLContext->CreateShader(std::move(ShaderSourceCode), ShaderType) };

        return OpenGLShaderIndex;
    }
}