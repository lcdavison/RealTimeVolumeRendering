#pragma once

#include <Windows.h>

#include <DirectXMath.h>

#include <array>
#include <memory>
#include <vector>
#include <filesystem>

#include "Vertex.h"

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
    class Texture2DSliceVolume
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

    public:
        Texture2DSliceVolume(OpenGL::Context* OpenGLContext);
        ~Texture2DSliceVolume();
     
        VOID DeleteResources(OpenGL::Context* OpenGLContext);

        VOID LoadVolumeDataset(OpenGL::Context* OpenGLContext, std::filesystem::path DatasetFilePath);

        VOID RenderSliceStackPositiveX(OpenGL::Context* OpenGLContext);
        VOID RenderSliceStackNegativeX(OpenGL::Context* OpenGLContext);

        VOID RenderSliceStackPositiveY(OpenGL::Context* OpenGLContext);
        VOID RenderSliceStackNegativeY(OpenGL::Context* OpenGLContext);

        VOID RenderSliceStackPositiveZ(OpenGL::Context* OpenGLContext);
        VOID RenderSliceStackNegativeZ(OpenGL::Context* OpenGLContext);

        DirectX::XMMATRIX ModelMatrix() const;

    private:

        VOID CreateResources(OpenGL::Context* OpenGLContext);

        VOID CreateVolumeSlices(OpenGL::Context* OpenGLContext);

        VOID CreateVolumeSlicesXAxis(OpenGL::Context* OpenGLContext);
        VOID CreateVolumeSlicesYAxis(OpenGL::Context* OpenGLContext);
        VOID CreateVolumeSlicesZAxis(OpenGL::Context* OpenGLContext);

        VOID RenderVolumeSlice(OpenGL::Context* OpenGLContext, const VolumeSlice& VolumeSlice);
    };
}