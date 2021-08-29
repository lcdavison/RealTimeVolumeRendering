#pragma once

#include <Windows.h>

namespace RTVR
{
    namespace OpenGL
    {
        class Context;
    }

    class Camera;

    class IVolumeRenderer
    {
    public:
        virtual ~IVolumeRenderer() {}

        virtual VOID Render(OpenGL::Context* OpenGLContext, Camera* Camera) = 0;

        virtual VOID DeleteResources(OpenGL::Context* OpenGLContext) = 0;
    };
}