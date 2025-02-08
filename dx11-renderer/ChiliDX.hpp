#pragma once
#include <DirectXMath.h>

namespace ChiliDX
{
DirectX::XMFLOAT3 ExtractPitchYawRoll( DirectX::XMFLOAT4X4 mat )
{
    DirectX::XMFLOAT3 euler;

    euler.x = asinf( -mat._32 );    // Pitch
    if ( cosf( euler.x ) > 0.0001 ) // Not at poles
    {
        euler.y = atan2f( mat._31, mat._33 ); // Yaw
        euler.z = atan2f( mat._12, mat._22 ); // Roll
    }
    else
    {
        euler.y = 0.0f;                        // Yaw
        euler.z = atan2f( -mat._21, mat._11 ); // Roll
    }

    return euler;
}

DirectX::XMFLOAT3 ExtractScale( DirectX::XMFLOAT4X4 mat )
{
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float scaleZ = 1.0f;

    return DirectX::XMFLOAT3( scaleX, scaleY, scaleZ );
}

DirectX::XMFLOAT3 ExtractTranslation( DirectX::XMFLOAT4X4 mat )
{

    return DirectX::XMFLOAT3{ mat._41, mat._42, mat._43 };
}
} // namespace ChiliDX
