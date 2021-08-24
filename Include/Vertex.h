#pragma once

#include <DirectXMath.h>

namespace RTVR
{
    struct Vertex
    {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT2 TextureCoordinate;
    };
}