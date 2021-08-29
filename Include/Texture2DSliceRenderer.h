#pragma once

#include "IVolumeRenderer.h"
#include "Texture2DSliceVolume.h"

#include <DirectXMath.h>

#include <memory>
#include <array>

namespace RTVR
{
    enum class Axis : UINT8
    {
        XAxis,
        YAxis,
        ZAxis
    };

    /* This renderer implements the 2D Slice rendering algorithm from Real-Time Volume Graphics by Klaus Engel et al. */
    class Texture2DSliceRenderer final : public IVolumeRenderer
    {
        std::unique_ptr<RTVR::Modelling::Texture2DSliceVolume> Volume_;
        std::unique_ptr<RTVR::OpenGL::Shader> SliceVolumeShader_;

    public:
        Texture2DSliceRenderer(OpenGL::Context* OpenGLContext);
        ~Texture2DSliceRenderer();
        
        VOID Render(OpenGL::Context* OpenGLContext, Camera* Camera) override final;

        VOID DeleteResources(OpenGL::Context* OpenGLContext) override final;

    private:
        VOID SetupShaderUniforms(OpenGL::Context* OpenGLContext, Camera* Camera);

        DirectX::XMFLOAT4 CalculateViewDirectionInVolumeSpace(Camera* Camera);

        Axis CalculateMaximumAxis(DirectX::XMFLOAT4 Vector);
    };
}