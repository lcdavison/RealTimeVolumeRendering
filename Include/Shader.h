#pragma once

#include <Windows.h>

#include "OpenGLContext.h"

#include <DirectXMath.h>

#include <string>
#include <filesystem>

namespace RTVR::OpenGL
{
    class Context;

    class Shader
    {
        UINT OpenGLShaderProgramID_;

    public:
        Shader(Context* OpenGLContext, const std::string& VertexShaderFilePath, const std::string& PixelShaderFilePath);

        VOID Bind(Context* OpenGLContext);
        VOID UnBind(Context* OpenGLContext);

        VOID SetUniformMatrix4x4(Context* OpenGLContext, const std::string& UniformName, const DirectX::XMFLOAT4X4& Matrix);

    private:
        UINT LoadAndCompileShaderFromFile(Context* OpenGLContext, const std::filesystem::path& ShaderFilePath, ShaderType ShaderType);
    };
}