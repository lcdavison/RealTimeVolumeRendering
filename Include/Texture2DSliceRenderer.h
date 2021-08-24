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
        DirectX::XMFLOAT4X4 ProjectionMatrix_;
        DirectX::XMFLOAT4X4 ViewMatrix_;

        std::unique_ptr<RTVR::Modelling::Texture2DSliceVolume> Volume_;
        std::unique_ptr<RTVR::OpenGL::Shader> SliceVolumeShader_;

    public:
        Texture2DSliceRenderer(OpenGL::Context* OpenGLContext);
        ~Texture2DSliceRenderer();
        
        VOID Render(OpenGL::Context* OpenGLContext) override final;

        VOID DeleteResources(OpenGL::Context* OpenGLContext) override final;

    private:
        void UpdateCamera();

        DirectX::XMFLOAT4 CalculateViewDirectionInVolumeSpace();

        Axis CalculateMaximumAxis(DirectX::XMFLOAT4 Vector);
    };
}