#pragma once

#include <Windows.h>

#include <DirectXMath.h>

namespace RTVR
{
    class Camera
    {
        DirectX::XMFLOAT4X4 ProjectionMatrix_;
        DirectX::XMFLOAT4X4 WorldToCameraSpaceMatrix_;

        DirectX::XMFLOAT4 PositionInWorldSpace_;

        DirectX::XMFLOAT4 ForwardDirectionInWorldSpace_;
        DirectX::XMFLOAT4 RightDirectionInWorldSpace_;
        DirectX::XMFLOAT4 UpDirectionInWorldSpace_;

        FLOAT VerticalFieldOfViewInRadians_;
        FLOAT ScreenAspectRatio_;
        FLOAT NearZValue_;
        FLOAT FarZValue_;

        bool IsWorldToCameraSpaceMatrixDirty_;

    public:
        Camera();
        Camera(DirectX::XMFLOAT3 Position, FLOAT VerticalFieldOfViewInRadians, FLOAT ScreenAspectRatio, FLOAT NearZValue, FLOAT FarZValue);

        VOID UpdateViewMatrix();

        VOID SetPosition(DirectX::XMFLOAT3 NewPosition);
        VOID LookAtTargetPoint(DirectX::XMFLOAT3 NewPositionInWorldSpace, DirectX::XMFLOAT3 TargetPositionInWorldSpace, DirectX::XMFLOAT3 UpAxisInWorldSpace);

        const DirectX::XMFLOAT4X4& ViewMatrix() const;
        const DirectX::XMFLOAT4X4& ProjectionMatrix() const;
    };
}