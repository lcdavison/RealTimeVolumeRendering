#include "Texture2DSliceRenderer.h"

#include "OpenGLContext.h"
#include "Texture2DSliceVolume.h"
#include "Shader.h"

#undef max

#include <algorithm>

using namespace DirectX;

extern FLOAT CameraAltitude;
extern FLOAT CameraAzimuth;
extern FLOAT CameraDistance;

namespace RTVR
{
    Texture2DSliceRenderer::Texture2DSliceRenderer(OpenGL::Context* OpenGLContext)
        : SliceVolumeShader_ { new OpenGL::Shader{ OpenGLContext,  "Shaders/Vertex.glsl", "Shaders/Pixel.glsl" } }
        , Volume_ { new RTVR::Modelling::Texture2DSliceVolume(OpenGLContext) }
        , ProjectionMatrix_ { }
        , ViewMatrix_ {}
    {
        Volume_->LoadVolumeDataset(OpenGLContext, "Datasets/Funky Ball/C60.vol");

        XMMATRIX ProjectionMatrix = XMMatrixPerspectiveFovRH(XMConvertToRadians(90.0f), 1920.0f / 1080.0f, 0.0001f, 1000.0f);
        XMStoreFloat4x4(&ProjectionMatrix_, ProjectionMatrix);

        SliceVolumeShader_->SetUniformMatrix4x4(OpenGLContext, "ProjectionMatrix", ProjectionMatrix_);
    }

    Texture2DSliceRenderer::~Texture2DSliceRenderer()
    {
    }

    VOID Texture2DSliceRenderer::DeleteResources(OpenGL::Context* OpenGLContext)
    {
        Volume_->DeleteResources(OpenGLContext);
    }

    VOID Texture2DSliceRenderer::Render(OpenGL::Context* OpenGLContext)
    {
        OpenGLContext->SetBlendFunction(OpenGL::BlendFactor::One, OpenGL::BlendFactor::OneMinusSourceAlpha);

        SliceVolumeShader_->Bind(OpenGLContext);

        UpdateCamera();
        SliceVolumeShader_->SetUniformMatrix4x4(OpenGLContext, "ViewMatrix", ViewMatrix_);

        XMFLOAT4 ViewDirectionInVolumeSpace = { CalculateViewDirectionInVolumeSpace() };

        Axis MaximumViewAxis = { CalculateMaximumAxis(ViewDirectionInVolumeSpace) };

        switch (MaximumViewAxis)
        {
        case Axis::XAxis:
            {
                if (ViewDirectionInVolumeSpace.x > 0.0f)
                {
                    Volume_->RenderSliceStackPositiveX(OpenGLContext);
                }
                else
                {
                    Volume_->RenderSliceStackNegativeX(OpenGLContext);
                }
            }
            break;
        case Axis::YAxis:
            {
                if (ViewDirectionInVolumeSpace.y > 0.0f)
                {
                    Volume_->RenderSliceStackPositiveY(OpenGLContext);
                }
                else
                {
                    Volume_->RenderSliceStackNegativeY(OpenGLContext);
                }
            }
            break;
        case Axis::ZAxis:
            {
                if (ViewDirectionInVolumeSpace.z > 0.0f)
                {
                    Volume_->RenderSliceStackPositiveZ(OpenGLContext);
                }
                else
                {
                    Volume_->RenderSliceStackNegativeZ(OpenGLContext);
                }
            }
            break;
        }

        SliceVolumeShader_->UnBind(OpenGLContext);
    }

    void Texture2DSliceRenderer::UpdateCamera()
    {
        XMVECTOR CameraOffset = { XMVectorSet(CameraDistance * std::cos(CameraAzimuth) * std::sin(CameraAltitude),
                                              CameraDistance * std::cos(CameraAltitude),
                                              CameraDistance * std::sin(CameraAzimuth) * std::sin(CameraAltitude), 0.0f) };

        XMVECTOR CameraPosition = { XMVectorAdd(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), CameraOffset) };

        XMMATRIX ViewMatrix = { XMMatrixLookAtRH(CameraPosition,
                                                 XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
                                                 XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)) };
        XMStoreFloat4x4(&ViewMatrix_, ViewMatrix);
    }

    XMFLOAT4 Texture2DSliceRenderer::CalculateViewDirectionInVolumeSpace()
    {
        XMMATRIX ModelViewMatrix = { XMMatrixMultiply(Volume_->ModelMatrix(),
                                                      XMLoadFloat4x4(&ViewMatrix_)) };

        XMVECTOR ModelViewMatrixDeterminant = { XMMatrixDeterminant(ModelViewMatrix) };
        XMMATRIX ModelViewInverseMatrix = { XMMatrixInverse(&ModelViewMatrixDeterminant, ModelViewMatrix) };

        XMFLOAT4 ViewDirectionInViewSpace = { 0.0f, 0.0f, -1.0f, 0.0f };

        XMFLOAT4 ViewDirectionInVolumeSpace;
        XMStoreFloat4(&ViewDirectionInVolumeSpace, XMVector4Transform(XMLoadFloat4(&ViewDirectionInViewSpace), ModelViewInverseMatrix));

        return ViewDirectionInVolumeSpace;
    }

    Axis Texture2DSliceRenderer::CalculateMaximumAxis(XMFLOAT4 Vector)
    {
        FLOAT AbsoluteXValue = { std::abs(Vector.x) };
        FLOAT AbsoluteYValue = { std::abs(Vector.y) };
        FLOAT AbsoluteZValue = { std::abs(Vector.z) };

        FLOAT LargestValue = std::max(AbsoluteXValue, std::max(AbsoluteYValue, AbsoluteZValue));

        if (LargestValue == AbsoluteYValue)
        {
            return Axis::YAxis;
        }
        else if (LargestValue == AbsoluteXValue)
        {
            return Axis::XAxis;
        }
        else if (LargestValue == AbsoluteZValue)
        {
            return Axis::ZAxis;
        }
    }
}