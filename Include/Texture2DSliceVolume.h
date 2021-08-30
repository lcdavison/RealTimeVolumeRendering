#pragma once

#include <Windows.h>

#include <DirectXMath.h>

#include <array>
#include <memory>
#include <vector>
#include <filesystem>

#include "IVolumeRenderer.h"
#include "Vertex.h"

namespace RTVR
{
    class Camera;
}

namespace RTVR::OpenGL
{
    class Context;
    class Shader;
}

namespace RTVR::IO
{
    struct Dataset;
}

namespace RTVR::Modelling
{
    class Texture2DSliceVolume : public IVolume
    {
        struct VolumeSlice
        {
            UINT OpenGLTextureID;
        };
        
        static const UINT VertexCountPerSlice = { 6 };

        std::vector<VolumeSlice> SliceStackXAxis_;
        std::vector<VolumeSlice> SliceStackYAxis_;
        std::vector<VolumeSlice> SliceStackZAxis_;

        std::vector<Vertex> CurrentSliceVertices_;

        DirectX::XMFLOAT4X4 ModelMatrix_;

        std::unique_ptr<IO::Dataset> VolumeTexture_;

        UINT SliceCountXAxis_;
        UINT SliceCountYAxis_;
        UINT SliceCountZAxis_;
        
        UINT OpenGLVertexArrayID_;
        UINT OpenGLVertexBufferID_;

        std::vector<UINT> SliceStackVertexBufferIDList_;

    public:
        Texture2DSliceVolume(OpenGL::Context* OpenGLContext);
        ~Texture2DSliceVolume();
     
        VOID DeleteResources(OpenGL::Context* OpenGLContext);

        VOID Render(OpenGL::Context* OpenGLContext, Camera* Camera) override final;

        VOID LoadVolumeDataset(OpenGL::Context* OpenGLContext, std::filesystem::path DatasetFilePath);

        DirectX::XMMATRIX ModelMatrix() const;

    private:

        VOID CreateResources(OpenGL::Context* OpenGLContext);

        VOID CreateVolumeSlices(OpenGL::Context* OpenGLContext);

        VOID ConstructVolumeSliceTextures(OpenGL::Context* OpenGLContext, Axis VolumeSliceStackAxis, UINT64 SliceWidth, UINT64 SliceHeight, UINT64& CurrentDepthIndex, VolumeSlice& VolumeSlice);
        UINT64 CalculateVolumeTextureIndex(Axis VolumeSliceStackAxis, UINT64 Column, UINT64 Row, UINT64 Depth, UINT64 SliceWidth, UINT64 SliceHeight);

        VOID RenderSliceStackPositiveX(OpenGL::Context* OpenGLContext);
        VOID RenderSliceStackNegativeX(OpenGL::Context* OpenGLContext);

        VOID RenderSliceStackPositiveY(OpenGL::Context* OpenGLContext);
        VOID RenderSliceStackNegativeY(OpenGL::Context* OpenGLContext);

        VOID RenderSliceStackPositiveZ(OpenGL::Context* OpenGLContext);
        VOID RenderSliceStackNegativeZ(OpenGL::Context* OpenGLContext);

        VOID RenderVolumeSlice(OpenGL::Context* OpenGLContext, const VolumeSlice& VolumeSlice);

        DirectX::XMFLOAT4 CalculateViewDirectionInVolumeSpace(Camera* Camera);

        Axis CalculateMaximumAxis(DirectX::XMFLOAT4 Vector);
    };
}