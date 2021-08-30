#include "Texture2DSliceRenderer.h"

#include "OpenGLContext.h"
#include "Texture2DSliceVolume.h"
#include "Shader.h"
#include "Camera.h"

#undef max

#include <algorithm>

using namespace DirectX;

namespace RTVR
{
    Texture2DSliceRenderer::Texture2DSliceRenderer(OpenGL::Context* OpenGLContext)
        : SliceVolumeShader_ { new OpenGL::Shader{ OpenGLContext,  "Shaders/Vertex.glsl", "Shaders/Pixel.glsl" } }
        , Volume_ { new RTVR::Modelling::Texture2DSliceVolume(OpenGLContext) }
    {
        Volume_->LoadVolumeDataset(OpenGLContext, "Datasets/Funky Ball/C60.vol");
    }

    Texture2DSliceRenderer::~Texture2DSliceRenderer()
    {
    }

    VOID Texture2DSliceRenderer::DeleteResources(OpenGL::Context* OpenGLContext)
    {
        Volume_->DeleteResources(OpenGLContext);
    }

    VOID Texture2DSliceRenderer::Render(OpenGL::Context* OpenGLContext, Camera* Camera)
    {
        SliceVolumeShader_->Bind(OpenGLContext);
        SetupShaderUniforms(OpenGLContext, Camera);

        Volume_->Render(OpenGLContext, Camera);

        SliceVolumeShader_->UnBind(OpenGLContext);
    }

    VOID Texture2DSliceRenderer::SetupShaderUniforms(OpenGL::Context* OpenGLContext, Camera* Camera)
    {
        const XMFLOAT4X4& ProjectionMatrix = { Camera->ProjectionMatrix() };
        SliceVolumeShader_->SetUniformMatrix4x4(OpenGLContext, "ProjectionMatrix", ProjectionMatrix);

        const XMFLOAT4X4& ViewMatrix = { Camera->ViewMatrix() };
        SliceVolumeShader_->SetUniformMatrix4x4(OpenGLContext, "ViewMatrix", ViewMatrix);
    }
}