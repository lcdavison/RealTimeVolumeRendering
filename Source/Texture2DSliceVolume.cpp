#include "Texture2DSliceVolume.h"

#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

#include "OpenGLContext.h"
#include "DatasetLoader.h"
#include "Shader.h"
#include "Vertex.h"
#include "Camera.h"

#include <sstream>
#include <functional>

/* Get rid of Windows stuff */
#undef max
#undef min

using namespace DirectX;

namespace RTVR::Modelling
{
    Texture2DSliceVolume::Texture2DSliceVolume(OpenGL::Context* OpenGLContext)
        : VolumeTexture_ { new RTVR::IO::Dataset {} }
        , CurrentSliceVertices_ { VertexCountPerSlice }
    {
        CreateResources(OpenGLContext);

        XMStoreFloat4x4(&ModelMatrix_, XMMatrixIdentity());
    }

    Texture2DSliceVolume::~Texture2DSliceVolume()
    {
    }

    VOID Texture2DSliceVolume::CreateResources(OpenGL::Context* OpenGLContext)
    {
        constexpr SIZE_T VertexBufferSizeInBytes = { (VertexCountPerSlice * sizeof(Vertex)) };

        OpenGLVertexArrayID_ = OpenGLContext->CreateVertexArray();

        OpenGLVertexBufferID_ = OpenGLContext->CreateBuffer(VertexBufferSizeInBytes, OpenGL::BufferType::Dynamic);

        OpenGLContext->CreateVertexAttribute(OpenGLVertexArrayID_, OpenGL::VertexAttribute::Position, OpenGLVertexBufferID_, 3, OpenGL::DataType::Float, sizeof(Vertex), 0, false);
        OpenGLContext->CreateVertexAttribute(OpenGLVertexArrayID_, OpenGL::VertexAttribute::TextureCoordinate, OpenGLVertexBufferID_, 2, OpenGL::DataType::Float, sizeof(Vertex), sizeof(XMFLOAT3), false);
    }

    VOID Texture2DSliceVolume::LoadVolumeDataset(OpenGL::Context* OpenGLContext, std::filesystem::path DatasetFilePath)
    {
        IO::DatasetLoader DatasetLoader = { DatasetFilePath };
        DatasetLoader.Read(*VolumeTexture_);

        SliceCountXAxis_ = VolumeTexture_->GridResolutionX;
        SliceCountYAxis_ = VolumeTexture_->GridResolutionY;
        SliceCountZAxis_ = VolumeTexture_->GridResolutionZ;

        SliceStackXAxis_.resize(SliceCountXAxis_);
        SliceStackYAxis_.resize(SliceCountYAxis_);
        SliceStackZAxis_.resize(SliceCountZAxis_);

        CreateVolumeSlices(OpenGLContext);

        VolumeTexture_.reset();
    }

    VOID Texture2DSliceVolume::CreateVolumeSlices(OpenGL::Context* OpenGLContext)
    {
        UINT64 CurrentDepthIndex = { 0 };
        std::for_each(SliceStackXAxis_.begin(), SliceStackXAxis_.end(), 
                      std::bind(&Texture2DSliceVolume::ConstructVolumeSliceTextures, this, 
                                OpenGLContext, 
                                Axis::X, 
                                VolumeTexture_->GridResolutionZ, VolumeTexture_->GridResolutionY, 
                                CurrentDepthIndex, std::placeholders::_1));

        CurrentDepthIndex = 0;
        std::for_each(SliceStackYAxis_.begin(), SliceStackYAxis_.end(),
                      std::bind(&Texture2DSliceVolume::ConstructVolumeSliceTextures, this,
                                OpenGLContext,
                                Axis::Y,
                                VolumeTexture_->GridResolutionX, VolumeTexture_->GridResolutionZ,
                                CurrentDepthIndex, std::placeholders::_1));

        CurrentDepthIndex = 0;
        std::for_each(SliceStackZAxis_.begin(), SliceStackZAxis_.end(),
                      std::bind(&Texture2DSliceVolume::ConstructVolumeSliceTextures, this,
                                OpenGLContext,
                                Axis::Z,
                                VolumeTexture_->GridResolutionX, VolumeTexture_->GridResolutionY,
                                CurrentDepthIndex, std::placeholders::_1));
    }

    VOID Texture2DSliceVolume::ConstructVolumeSliceTextures(OpenGL::Context* OpenGLContext, Axis VolumeSliceStackAxis, UINT64 SliceWidth, UINT64 SliceHeight, UINT64& CurrentDepthIndex, VolumeSlice& VolumeSlice)
    {
        std::vector<std::byte> VolumeSliceTextureData = {};

        SIZE_T VolumeTextureSizeInTexels = { SliceWidth * SliceHeight };
        VolumeSliceTextureData.resize(VolumeTextureSizeInTexels);

        for (UINT64 CurrentRowIndex = { 0 }; CurrentRowIndex < SliceHeight; CurrentRowIndex++)
        {
            for (UINT64 CurrentColumnIndex = { 0 }; CurrentColumnIndex < SliceWidth; CurrentColumnIndex++)
            {
                UINT64 VolumeTextureIndex = { CalculateVolumeTextureIndex(VolumeSliceStackAxis, CurrentColumnIndex, CurrentRowIndex, CurrentDepthIndex, SliceWidth, SliceHeight) };

                VolumeSliceTextureData [CurrentRowIndex * SliceWidth + CurrentColumnIndex] = VolumeTexture_->VolumeData [VolumeTextureIndex];
            }
        }

        VolumeSlice.OpenGLTextureID = OpenGLContext->CreateTexture2D(SliceWidth, SliceHeight, VolumeSliceTextureData.data(), 1, OpenGL::TexelFormat::GrayScale, OpenGL::DataType::UnsignedByte);

        ++CurrentDepthIndex;
    }

    UINT64 Texture2DSliceVolume::CalculateVolumeTextureIndex(Axis PrimaryAxis, UINT64 Column, UINT64 Row, UINT64 Depth, UINT64 SliceWidth, UINT64 SliceHeight)
    {
        switch (PrimaryAxis)
        {
        case Axis::X:
            {
                return (Column * SliceHeight + Row) * SliceWidth + Depth;
            }
        case Axis::Y:
            {
                return (Row * SliceHeight + Depth) * SliceWidth + Column;
            }
        case Axis::Z:
            {
                return (Depth * SliceHeight + Row) * SliceWidth + Column;
            }
        }
    }

    VOID Texture2DSliceVolume::DeleteResources(OpenGL::Context* OpenGLContext)
    {
        OpenGLContext->DeleteVertexArray(OpenGLVertexArrayID_);

        OpenGLContext->DeleteBuffer(OpenGLVertexBufferID_);

        for (const VolumeSlice& VolumeSlice : SliceStackXAxis_)
        {
            OpenGLContext->DeleteTexture2D(VolumeSlice.OpenGLTextureID);
        }

        for (const VolumeSlice& VolumeSlice : SliceStackYAxis_)
        {
            OpenGLContext->DeleteTexture2D(VolumeSlice.OpenGLTextureID);
        }

        for (const VolumeSlice& VolumeSlice : SliceStackZAxis_)
        {
            OpenGLContext->DeleteTexture2D(VolumeSlice.OpenGLTextureID);
        }
    }

    VOID Texture2DSliceVolume::Render(OpenGL::Context* OpenGLContext, Camera* Camera)
    {
        OpenGLContext->SetBlendFunction(OpenGL::BlendFactor::One, OpenGL::BlendFactor::OneMinusSourceAlpha);

        XMFLOAT4 ViewDirectionInVolumeSpace = { CalculateViewDirectionInVolumeSpace(Camera) };

        Axis MaximumViewAxis = { CalculateMaximumAxis(ViewDirectionInVolumeSpace) };

        switch (MaximumViewAxis)
        {
        case Axis::X:
            {
                if (ViewDirectionInVolumeSpace.x > 0.0f)
                {
                    RenderSliceStackPositiveX(OpenGLContext);
                }
                else
                {
                    RenderSliceStackNegativeX(OpenGLContext);
                }
            }
            break;
        case Axis::Y:
            {
                if (ViewDirectionInVolumeSpace.y > 0.0f)
                {
                    RenderSliceStackPositiveY(OpenGLContext);
                }
                else
                {
                    RenderSliceStackNegativeY(OpenGLContext);
                }
            }
            break;
        case Axis::Z:
            {
                if (ViewDirectionInVolumeSpace.z > 0.0f)
                {
                    RenderSliceStackPositiveZ(OpenGLContext);
                }
                else
                {
                    RenderSliceStackNegativeZ(OpenGLContext);
                }
            }
            break;
        }
    }

    VOID Texture2DSliceVolume::RenderSliceStackPositiveX(OpenGL::Context* OpenGLContext)
    {
        DOUBLE CurrentPositionX = { 1.0 };
        DOUBLE PositionDecrementX = { 2.0 / SliceCountXAxis_ };
            
        OpenGLContext->EnableVertexAttributes(OpenGLVertexArrayID_, { OpenGL::VertexAttribute::Position, OpenGL::VertexAttribute::TextureCoordinate });

        for (INT CurrentSliceIndex = { static_cast<INT>(SliceCountXAxis_) - 1 }; CurrentSliceIndex > -1; CurrentSliceIndex--)
        {
            const VolumeSlice& CurrentSlice = { SliceStackXAxis_ [CurrentSliceIndex] };

            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), -1.0f, -1.0f }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), -1.0f, 1.0f }, { 0.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), 1.0f, -1.0f }, { 1.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), 1.0f, -1.0f }, { 1.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), -1.0f, 1.0f }, { 0.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), 1.0f, 1.0f }, { 0.0f, 1.0f } });

            OpenGLContext->UpdateBuffer(OpenGLVertexBufferID_, CurrentSliceVertices_.data(), CurrentSliceVertices_.size() * sizeof(Vertex));

            RenderVolumeSlice(OpenGLContext, CurrentSlice);

            CurrentPositionX -= PositionDecrementX;

            CurrentSliceVertices_.clear();
        }

        OpenGLContext->DisableVertexAttributes(OpenGLVertexArrayID_, { OpenGL::VertexAttribute::Position, OpenGL::VertexAttribute::TextureCoordinate });
    }

    VOID Texture2DSliceVolume::RenderSliceStackNegativeX(OpenGL::Context* OpenGLContext)
    {
        DOUBLE CurrentPositionX = { -1.0 };
        DOUBLE PositionIncrementX = { 2.0 / SliceCountXAxis_ };

        OpenGLContext->EnableVertexAttributes(OpenGLVertexArrayID_, { OpenGL::VertexAttribute::Position, OpenGL::VertexAttribute::TextureCoordinate });

        for (INT CurrentSliceIndex = { 0 }; CurrentSliceIndex < SliceCountXAxis_; CurrentSliceIndex++)
        {
            const VolumeSlice& CurrentSlice = { SliceStackXAxis_ [CurrentSliceIndex] };

            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), -1.0f, 1.0f }, { 0.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), -1.0f, -1.0f }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), 1.0f, 1.0f }, { 0.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), 1.0f, 1.0f }, { 0.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), -1.0f, -1.0f }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), 1.0f, -1.0f }, { 1.0f, 1.0f } });

            OpenGLContext->UpdateBuffer(OpenGLVertexBufferID_, CurrentSliceVertices_.data(), CurrentSliceVertices_.size() * sizeof(Vertex));

            RenderVolumeSlice(OpenGLContext, CurrentSlice);

            CurrentPositionX += PositionIncrementX;

            CurrentSliceVertices_.clear();
        }

        OpenGLContext->DisableVertexAttributes(OpenGLVertexArrayID_, { OpenGL::VertexAttribute::Position, OpenGL::VertexAttribute::TextureCoordinate });
    }

    VOID Texture2DSliceVolume::RenderSliceStackPositiveY(OpenGL::Context* OpenGLContext)
    {
        DOUBLE CurrentPositionY = { 1.0 };
        DOUBLE PositionDecrementY = { 2.0 / SliceCountYAxis_ };
        
        OpenGLContext->EnableVertexAttributes(OpenGLVertexArrayID_, { OpenGL::VertexAttribute::Position, OpenGL::VertexAttribute::TextureCoordinate });

        for (INT CurrentSliceIndex = { static_cast<INT>(SliceCountYAxis_) - 1 }; CurrentSliceIndex > -1; CurrentSliceIndex--)
        {
            const VolumeSlice& CurrentSlice = { SliceStackYAxis_ [CurrentSliceIndex] };

            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, static_cast<FLOAT>(CurrentPositionY), 1.0f }, { 1.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, static_cast<FLOAT>(CurrentPositionY), 1.0f }, { 0.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, static_cast<FLOAT>(CurrentPositionY), -1.0f }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, static_cast<FLOAT>(CurrentPositionY), -1.0f }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, static_cast<FLOAT>(CurrentPositionY), 1.0f }, { 0.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, static_cast<FLOAT>(CurrentPositionY), -1.0f }, { 0.0f, 0.0f } });

            OpenGLContext->UpdateBuffer(OpenGLVertexBufferID_, CurrentSliceVertices_.data(), CurrentSliceVertices_.size() * sizeof(Vertex));

            RenderVolumeSlice(OpenGLContext, CurrentSlice);

            CurrentPositionY -= PositionDecrementY;

            CurrentSliceVertices_.clear();
        }

        OpenGLContext->DisableVertexAttributes(OpenGLVertexArrayID_, { OpenGL::VertexAttribute::Position, OpenGL::VertexAttribute::TextureCoordinate });
    }

    VOID Texture2DSliceVolume::RenderSliceStackNegativeY(OpenGL::Context* OpenGLContext)
    {
        DOUBLE CurrentPositionY = { -1.0 };
        DOUBLE PositionIncrementY = { 2.0 / SliceCountYAxis_ };

        OpenGLContext->EnableVertexAttributes(OpenGLVertexArrayID_, { OpenGL::VertexAttribute::Position, OpenGL::VertexAttribute::TextureCoordinate });

        for (INT CurrentSliceIndex = { 0 }; CurrentSliceIndex < SliceCountYAxis_; CurrentSliceIndex++)
        {
            const VolumeSlice& CurrentSlice = { SliceStackYAxis_ [CurrentSliceIndex] };

            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, static_cast<FLOAT>(CurrentPositionY), -1.0f }, { 0.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, static_cast<FLOAT>(CurrentPositionY), -1.0f }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, static_cast<FLOAT>(CurrentPositionY), 1.0f }, { 0.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, static_cast<FLOAT>(CurrentPositionY), 1.0f }, { 0.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, static_cast<FLOAT>(CurrentPositionY), -1.0f }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, static_cast<FLOAT>(CurrentPositionY), 1.0f }, { 1.0f, 1.0f } });

            OpenGLContext->UpdateBuffer(OpenGLVertexBufferID_, CurrentSliceVertices_.data(), CurrentSliceVertices_.size() * sizeof(Vertex));

            RenderVolumeSlice(OpenGLContext, CurrentSlice);

            CurrentPositionY += PositionIncrementY;

            CurrentSliceVertices_.clear();
        }

        OpenGLContext->DisableVertexAttributes(OpenGLVertexArrayID_, { OpenGL::VertexAttribute::Position, OpenGL::VertexAttribute::TextureCoordinate });
    }

    VOID Texture2DSliceVolume::RenderSliceStackPositiveZ(OpenGL::Context* OpenGLContext)
    {
        DOUBLE CurrentPositionZ = { 1.0 };
        DOUBLE PositionDecrementZ = { 2.0 / SliceCountZAxis_ };

        OpenGLContext->EnableVertexAttributes(OpenGLVertexArrayID_, { OpenGL::VertexAttribute::Position, OpenGL::VertexAttribute::TextureCoordinate });

        for (INT CurrentSliceIndex = { static_cast<INT>(SliceCountZAxis_) - 1 }; CurrentSliceIndex > -1; CurrentSliceIndex--)
        {
            const VolumeSlice& CurrentSlice = { SliceStackZAxis_ [CurrentSliceIndex] };

            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, -1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, -1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 0.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, 1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 1.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, 1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 1.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, -1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 0.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, 1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 0.0f, 1.0f } });

            OpenGLContext->UpdateBuffer(OpenGLVertexBufferID_, CurrentSliceVertices_.data(), CurrentSliceVertices_.size() * sizeof(Vertex));

            RenderVolumeSlice(OpenGLContext, CurrentSlice);

            CurrentPositionZ -= PositionDecrementZ;

            CurrentSliceVertices_.clear();
        }

        OpenGLContext->DisableVertexAttributes(OpenGLVertexArrayID_, { OpenGL::VertexAttribute::Position, OpenGL::VertexAttribute::TextureCoordinate });
    }

    VOID Texture2DSliceVolume::RenderSliceStackNegativeZ(OpenGL::Context* OpenGLContext)
    {
        DOUBLE CurrentPositionZ = { -1.0 };
        DOUBLE PositionIncrementZ = { 2.0 / SliceCountZAxis_ };

        OpenGLContext->EnableVertexAttributes(OpenGLVertexArrayID_, { OpenGL::VertexAttribute::Position, OpenGL::VertexAttribute::TextureCoordinate });

        for (INT CurrentSliceIndex = { 0 }; CurrentSliceIndex < SliceCountZAxis_; CurrentSliceIndex++)
        {
            const VolumeSlice& CurrentSlice = { SliceStackZAxis_ [CurrentSliceIndex] };

            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, -1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 0.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, -1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, 1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 0.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, 1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 0.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, -1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, 1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 1.0f, 1.0f } });

            OpenGLContext->UpdateBuffer(OpenGLVertexBufferID_, CurrentSliceVertices_.data(), CurrentSliceVertices_.size() * sizeof(Vertex));

            RenderVolumeSlice(OpenGLContext, CurrentSlice);

            CurrentPositionZ += PositionIncrementZ;

            CurrentSliceVertices_.clear();
        }

        OpenGLContext->DisableVertexAttributes(OpenGLVertexArrayID_, { OpenGL::VertexAttribute::Position, OpenGL::VertexAttribute::TextureCoordinate });
    }

    VOID Texture2DSliceVolume::RenderVolumeSlice(OpenGL::Context* OpenGLContext, const VolumeSlice& VolumeSlice)
    {
        OpenGLContext->BindTexture2D(VolumeSlice.OpenGLTextureID, 0);

        OpenGLContext->DrawVertices(VertexCountPerSlice);
    }

    XMFLOAT4 Texture2DSliceVolume::CalculateViewDirectionInVolumeSpace(Camera* Camera)
    {
        const XMFLOAT4X4& ViewMatrix = { Camera->ViewMatrix() };

        XMMATRIX ModelViewMatrix = { XMMatrixMultiply(XMLoadFloat4x4(&ModelMatrix_),
                                                      XMLoadFloat4x4(&ViewMatrix)) };

        XMVECTOR ModelViewMatrixDeterminant = { XMMatrixDeterminant(ModelViewMatrix) };
        XMMATRIX ModelViewInverseMatrix = { XMMatrixInverse(&ModelViewMatrixDeterminant, ModelViewMatrix) };

        XMFLOAT4 ViewDirectionInViewSpace = { 0.0f, 0.0f, -1.0f, 0.0f };

        XMFLOAT4 ViewDirectionInVolumeSpace;
        XMStoreFloat4(&ViewDirectionInVolumeSpace, XMVector4Transform(XMLoadFloat4(&ViewDirectionInViewSpace), ModelViewInverseMatrix));

        return ViewDirectionInVolumeSpace;
    }

    Axis Texture2DSliceVolume::CalculateMaximumAxis(XMFLOAT4 Vector)
    {
        FLOAT AbsoluteXValue = { std::abs(Vector.x) };
        FLOAT AbsoluteYValue = { std::abs(Vector.y) };
        FLOAT AbsoluteZValue = { std::abs(Vector.z) };

        FLOAT LargestValue = std::max(AbsoluteXValue, std::max(AbsoluteYValue, AbsoluteZValue));

        if (LargestValue == AbsoluteYValue)
        {
            return Axis::Y;
        }
        else if (LargestValue == AbsoluteXValue)
        {
            return Axis::X;
        }
        else if (LargestValue == AbsoluteZValue)
        {
            return Axis::Z;
        }
    }

    XMMATRIX Texture2DSliceVolume::ModelMatrix() const
    {
        return XMMatrixMultiply(XMLoadFloat4x4(&ModelMatrix_), XMMatrixScaling(4.0f, 4.0f, 4.0f));
    }
}