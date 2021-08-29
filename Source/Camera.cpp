#include "Camera.h"

using namespace DirectX;

namespace RTVR
{
    Camera::Camera()
        : Camera(XMFLOAT3 { 0.0f, 0.0f, 0.0f }, XMConvertToRadians(60.0f), 1920.0f / 1080.0f, 0.0001f, 1000.0f)
    {
    }

    Camera::Camera(XMFLOAT3 Position, FLOAT VerticalFieldOfViewInRadians, FLOAT ScreenAspectRatio, FLOAT NearZValue, FLOAT FarZValue)
        : PositionInWorldSpace_ { Position.x, Position.y, Position.z, 1.0f }
        , RightDirectionInWorldSpace_ { 1.0f, 0.0f, 0.0, 0.0f }
        , UpDirectionInWorldSpace_ { 0.0f, 1.0f, 0.0f, 0.0f }
        , ForwardDirectionInWorldSpace_ { 0.0f, 0.0f, 1.0f, 0.0f }
        , WorldToCameraSpaceMatrix_ {}
        , IsWorldToCameraSpaceMatrixDirty_ { true }
        , VerticalFieldOfViewInRadians_ { VerticalFieldOfViewInRadians }
        , ScreenAspectRatio_ { ScreenAspectRatio }
        , NearZValue_ { NearZValue }
        , FarZValue_ { FarZValue }
    {
        XMStoreFloat4x4(&ProjectionMatrix_, XMMatrixPerspectiveFovRH(VerticalFieldOfViewInRadians_, ScreenAspectRatio_, NearZValue_, FarZValue_));
    }

    VOID Camera::UpdateViewMatrix()
    {
        if (IsWorldToCameraSpaceMatrixDirty_)
        {
            XMVECTOR XMPositionInWorldSpace = { XMLoadFloat4(&PositionInWorldSpace_) };

            XMVECTOR XMRightDirectionInWorldSpace = { XMLoadFloat4(&RightDirectionInWorldSpace_) };
            XMVECTOR XMUpDirectionInWorldSpace = { XMLoadFloat4(&UpDirectionInWorldSpace_) };
            XMVECTOR XMForwardDirectionInWorldSpace = { XMLoadFloat4(&ForwardDirectionInWorldSpace_) };

            XMForwardDirectionInWorldSpace = XMVector3Normalize(XMForwardDirectionInWorldSpace);
            XMUpDirectionInWorldSpace = XMVector3Normalize(XMVector3Cross(XMForwardDirectionInWorldSpace, XMRightDirectionInWorldSpace));
            XMRightDirectionInWorldSpace = XMVector3Cross(XMUpDirectionInWorldSpace, XMForwardDirectionInWorldSpace);

            XMStoreFloat4(&RightDirectionInWorldSpace_, XMRightDirectionInWorldSpace);
            XMStoreFloat4(&UpDirectionInWorldSpace_, XMUpDirectionInWorldSpace);
            XMStoreFloat4(&ForwardDirectionInWorldSpace_, XMForwardDirectionInWorldSpace);

            FLOAT TranslationX = { -1.0f * XMVectorGetX(XMVector3Dot(XMRightDirectionInWorldSpace, XMPositionInWorldSpace)) };
            FLOAT TranslationY = { -1.0f * XMVectorGetY(XMVector3Dot(XMUpDirectionInWorldSpace, XMPositionInWorldSpace)) };
            FLOAT TranslationZ = { -1.0f * XMVectorGetZ(XMVector3Dot(XMForwardDirectionInWorldSpace, XMPositionInWorldSpace)) };

            WorldToCameraSpaceMatrix_(0, 0) = RightDirectionInWorldSpace_.x;
            WorldToCameraSpaceMatrix_(1, 0) = RightDirectionInWorldSpace_.y;
            WorldToCameraSpaceMatrix_(2, 0) = RightDirectionInWorldSpace_.z;
            WorldToCameraSpaceMatrix_(3, 0) = TranslationX;

            WorldToCameraSpaceMatrix_(0, 1) = UpDirectionInWorldSpace_.x;
            WorldToCameraSpaceMatrix_(1, 1) = UpDirectionInWorldSpace_.y;
            WorldToCameraSpaceMatrix_(2, 1) = UpDirectionInWorldSpace_.z;
            WorldToCameraSpaceMatrix_(3, 1) = TranslationY;

            WorldToCameraSpaceMatrix_(0, 2) = ForwardDirectionInWorldSpace_.x;
            WorldToCameraSpaceMatrix_(1, 2) = ForwardDirectionInWorldSpace_.y;
            WorldToCameraSpaceMatrix_(2, 2) = ForwardDirectionInWorldSpace_.z;
            WorldToCameraSpaceMatrix_(3, 2) = TranslationZ;

            WorldToCameraSpaceMatrix_(0, 3) = 0.0f;
            WorldToCameraSpaceMatrix_(1, 3) = 0.0f;
            WorldToCameraSpaceMatrix_(2, 3) = 0.0f;
            WorldToCameraSpaceMatrix_(3, 3) = 1.0f;

            IsWorldToCameraSpaceMatrixDirty_ = false;
        }
    }

    VOID Camera::SetPosition(XMFLOAT3 NewPosition)
    {
        PositionInWorldSpace_ = XMFLOAT4 { NewPosition.x, NewPosition.y, NewPosition.z, 1.0f };

        IsWorldToCameraSpaceMatrixDirty_ = true;
    }

    VOID Camera::LookAtTargetPoint(XMFLOAT3 NewPositionInWorldSpace, XMFLOAT3 TargetPositionInWorldSpace, XMFLOAT3 UpAxisInWorldSpace)
    {
        XMVECTOR XMPositionInWorldSpace = { XMVectorSet(NewPositionInWorldSpace.x, NewPositionInWorldSpace.y, NewPositionInWorldSpace.z, 1.0f) };
        XMStoreFloat4(&PositionInWorldSpace_, XMPositionInWorldSpace);

        XMVECTOR XMTargetPositionInWorldSpace = { XMVectorSet(TargetPositionInWorldSpace.x, TargetPositionInWorldSpace.y, TargetPositionInWorldSpace.z, 1.0f) };
        XMVECTOR XMUpAxisInWorldSpace = { XMVectorSet(UpAxisInWorldSpace.x, UpAxisInWorldSpace.y, UpAxisInWorldSpace.z, 0.0f) };

        XMVECTOR XMForwardDirectionInWorldSpace = { XMVector3Normalize(XMVectorSubtract(XMPositionInWorldSpace, XMTargetPositionInWorldSpace)) };
        XMStoreFloat4(&ForwardDirectionInWorldSpace_, XMForwardDirectionInWorldSpace);

        XMVECTOR XMRightDirectionInWorldSpace = { XMVector3Normalize(XMVector3Cross(XMUpAxisInWorldSpace, XMForwardDirectionInWorldSpace)) };
        XMStoreFloat4(&RightDirectionInWorldSpace_, XMRightDirectionInWorldSpace);

        XMVECTOR XMUpDirectionInWorldSpace = { XMVector3Cross(XMForwardDirectionInWorldSpace, XMRightDirectionInWorldSpace) };
        XMStoreFloat4(&UpDirectionInWorldSpace_, XMUpDirectionInWorldSpace);

        IsWorldToCameraSpaceMatrixDirty_ = true;
    }

    const XMFLOAT4X4& Camera::ViewMatrix() const
    {
        return WorldToCameraSpaceMatrix_;
    }

    const XMFLOAT4X4& Camera::ProjectionMatrix() const
    {
        return ProjectionMatrix_;
    }
}