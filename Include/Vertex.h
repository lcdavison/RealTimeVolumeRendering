#pragma once

#include <Windows.h>
#include <DirectXMath.h>

namespace RTVR
{
    enum class Axis : UINT8
    {
        X,
        Y,
        Z
    };

    struct Vertex
    {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT2 TextureCoordinate;
    };
}