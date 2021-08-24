#include "Texture2DSliceVolume.h"

#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

#include "OpenGLContext.h"
#include "DatasetLoader.h"
#include "Shader.h"
#include "Vertex.h"

#include <sstream>

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
        CreateVolumeSlicesXAxis(OpenGLContext);
        CreateVolumeSlicesYAxis(OpenGLContext);
        CreateVolumeSlicesZAxis(OpenGLContext);
    }

    VOID Texture2DSliceVolume::CreateVolumeSlicesXAxis(OpenGL::Context* OpenGLContext)
    {
        UINT64 CurrentXIndex = { 0 };

        auto ProcessVolumeSlice = [this, OpenGLContext, &CurrentXIndex](VolumeSlice& Slice)
        {
            std::vector<std::byte> VolumeSliceTextureData = {};

            SIZE_T VolumeTextureSizeInTexels = { static_cast<UINT64>(VolumeTexture_->GridResolutionZ) * static_cast<UINT64>(VolumeTexture_->GridResolutionY) };
            VolumeSliceTextureData.resize(VolumeTextureSizeInTexels);

            for (UINT64 CurrentYIndex = { 0 }; CurrentYIndex < VolumeTexture_->GridResolutionY; CurrentYIndex++)
            {
                for (UINT64 CurrentZIndex = { 0 }; CurrentZIndex < VolumeTexture_->GridResolutionZ; CurrentZIndex++)
                {
                    SIZE_T VolumeDataIndex = { (CurrentZIndex * VolumeTexture_->GridResolutionY + CurrentYIndex) * VolumeTexture_->GridResolutionX + CurrentXIndex };

                    VolumeSliceTextureData [CurrentYIndex * (VolumeTexture_->GridResolutionZ) + CurrentZIndex] = VolumeTexture_->VolumeData [VolumeDataIndex];
                }
            }

            Slice.OpenGLTextureID = OpenGLContext->CreateTexture2D(VolumeTexture_->GridResolutionZ, VolumeTexture_->GridResolutionY, VolumeSliceTextureData.data(), 1, OpenGL::TexelFormat::GrayScale, OpenGL::DataType::UnsignedByte);

            ++CurrentXIndex;
        };

        std::for_each(SliceStackXAxis_.begin(), SliceStackXAxis_.end(), ProcessVolumeSlice);
    }

    VOID Texture2DSliceVolume::CreateVolumeSlicesYAxis(OpenGL::Context* OpenGLContext)
    {
        UINT64 CurrentYIndex = { 0 };

        auto ProcessVolumeSlice = [this, OpenGLContext, &CurrentYIndex](VolumeSlice& Slice)
        {
            std::vector<std::byte> VolumeSliceTextureData = {};

            SIZE_T VolumeTextureSizeInTexels = { static_cast<UINT64>(VolumeTexture_->GridResolutionX) * static_cast<UINT64>(VolumeTexture_->GridResolutionZ) };
            VolumeSliceTextureData.resize(VolumeTextureSizeInTexels);

            for (UINT64 CurrentZIndex = { 0 }; CurrentZIndex < VolumeTexture_->GridResolutionZ; CurrentZIndex++)
            {
                for (UINT64 CurrentXIndex = { 0 }; CurrentXIndex < VolumeTexture_->GridResolutionX; CurrentXIndex++)
                {
                    SIZE_T VolumeDataIndex = { (CurrentZIndex * VolumeTexture_->GridResolutionY + CurrentYIndex) * VolumeTexture_->GridResolutionX + CurrentXIndex };

                    VolumeSliceTextureData [CurrentZIndex * (VolumeTexture_->GridResolutionX) + CurrentXIndex] = VolumeTexture_->VolumeData [VolumeDataIndex];
                }
            }

            Slice.OpenGLTextureID = OpenGLContext->CreateTexture2D(VolumeTexture_->GridResolutionX, VolumeTexture_->GridResolutionZ, VolumeSliceTextureData.data(), 1, OpenGL::TexelFormat::GrayScale, OpenGL::DataType::UnsignedByte);

            ++CurrentYIndex;
        };

        std::for_each(SliceStackYAxis_.begin(), SliceStackYAxis_.end(), ProcessVolumeSlice);
    }

    VOID Texture2DSliceVolume::CreateVolumeSlicesZAxis(OpenGL::Context* OpenGLContext)
    {
        UINT64 CurrentZIndex = { 0 };

        auto ProcessVolumeSlice = [this, OpenGLContext, &CurrentZIndex](VolumeSlice& Slice)
        {
            std::vector<std::byte> VolumeSliceTextureData = {};

            SIZE_T VolumeTextureSizeInTexels = { static_cast<UINT64>(VolumeTexture_->GridResolutionX) * static_cast<UINT64>(VolumeTexture_->GridResolutionY) };
            VolumeSliceTextureData.resize(VolumeTextureSizeInTexels);

            for (UINT64 CurrentYIndex = { 0 }; CurrentYIndex < VolumeTexture_->GridResolutionY; CurrentYIndex++)
            {
                for (UINT64 CurrentXIndex = { 0 }; CurrentXIndex < VolumeTexture_->GridResolutionX; CurrentXIndex++)
                {
                    SIZE_T VolumeDataIndex = { (CurrentZIndex * VolumeTexture_->GridResolutionY + CurrentYIndex) * VolumeTexture_->GridResolutionX + CurrentXIndex };

                    VolumeSliceTextureData [CurrentYIndex * (VolumeTexture_->GridResolutionX) + CurrentXIndex] = VolumeTexture_->VolumeData [VolumeDataIndex];
                }
            }

            Slice.OpenGLTextureID = OpenGLContext->CreateTexture2D(VolumeTexture_->GridResolutionX, VolumeTexture_->GridResolutionY, VolumeSliceTextureData.data(), 1, OpenGL::TexelFormat::GrayScale, OpenGL::DataType::UnsignedByte);

            ++CurrentZIndex;
        };

        std::for_each(SliceStackZAxis_.begin(), SliceStackZAxis_.end(), ProcessVolumeSlice);
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

    VOID Texture2DSliceVolume::RenderSliceStackPositiveX(OpenGL::Context* OpenGLContext)
    {
        DOUBLE CurrentPositionX = { 1.0 };
        DOUBLE PositionDecrementX = { 2.0 / SliceCountXAxis_ };
            
        OpenGLContext->EnableVertexAttributes(OpenGLVertexArrayID_, { OpenGL::VertexAttribute::Position, OpenGL::VertexAttribute::TextureCoordinate });

        for (INT CurrentSliceIndex = { static_cast<INT>(SliceCountXAxis_) - 1 }; CurrentSliceIndex > -1; CurrentSliceIndex--)
        {
            const VolumeSlice& CurrentSlice = { SliceStackXAxis_ [CurrentSliceIndex] };

            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), -1.0f, -1.0f }, { 0.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), -1.0f, 1.0f }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), 1.0f, -1.0f }, { 0.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), 1.0f, -1.0f }, { 0.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), -1.0f, 1.0f }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { static_cast<FLOAT>(CurrentPositionX), 1.0f, 1.0f }, { 1.0f, 1.0f } });

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

            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, static_cast<FLOAT>(CurrentPositionY), 1.0f }, { 0.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, static_cast<FLOAT>(CurrentPositionY), 1.0f }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, static_cast<FLOAT>(CurrentPositionY), -1.0f }, { 0.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, static_cast<FLOAT>(CurrentPositionY), -1.0f }, { 0.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, static_cast<FLOAT>(CurrentPositionY), 1.0f }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, static_cast<FLOAT>(CurrentPositionY), -1.0f }, { 1.0f, 1.0f } });

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

            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, -1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 0.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, -1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, 1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 0.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { 1.0f, 1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 0.0f, 1.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, -1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 1.0f, 0.0f } });
            CurrentSliceVertices_.emplace_back(Vertex { { -1.0f, 1.0f, static_cast<FLOAT>(CurrentPositionZ) }, { 1.0f, 1.0f } });

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

    XMMATRIX Texture2DSliceVolume::ModelMatrix() const
    {
        return XMMatrixMultiply(XMLoadFloat4x4(&ModelMatrix_), XMMatrixScaling(4.0f, 4.0f, 4.0f));
    }
}