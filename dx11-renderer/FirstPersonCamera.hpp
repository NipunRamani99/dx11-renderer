#pragma once
#include "Graphics.hpp"
#include "ConstantBuffers.hpp"
#include <memory>
#include "WindowsConstants.h"
#include "Keyboard.hpp"
#include "Mouse.hpp"

class FirstPersonCamera
{

private:
	DirectX::XMFLOAT3 _front;
	DirectX::XMFLOAT3 _up;
	DirectX::XMFLOAT3 _cameraPos;
	float _cameraSpeed = 0.10f;
	DirectX::XMFLOAT2 _cursorPos;
	DirectX::XMFLOAT2 _prevCursorPos;

public:
	float _pitch = 0.0f;
	float _yaw = 90.0f;
	FirstPersonCamera()
		:
		_cursorPos(float(SCREEN_WIDTH >> 1), float(SCREEN_HEIGHT >> 1)),
		_prevCursorPos(_cursorPos),
		_cameraPos(0.0f, 10.0f, -7.0f),
		_up(0.0f, 1.0f, 0.0f),
		_front(0.0f, 0.0f, 0.0f)
	{
		// Convert pitch and yaw to radians for trigonometric functions
		float pitchRad = DirectX::XMConvertToRadians(_pitch);
		float yawRad = DirectX::XMConvertToRadians(_yaw);

		// Update the front vector based on the yaw and pitch
		_front.x = cosf(pitchRad) * cosf(yawRad);
		_front.y = sinf(pitchRad);
		_front.z = cosf(pitchRad) * sinf(yawRad);

		// Normalize the front vector
		auto vec = DirectX::XMLoadFloat3(&_front);
		vec = DirectX::XMVector3Normalize(vec);
		DirectX::XMStoreFloat3(&_front, vec);
	}

	DirectX::XMFLOAT3 GetPos()
	{
		return _cameraPos;
	}

	DirectX::XMFLOAT3 GetDir()
	{
		return _front;
	}

	DirectX::XMMATRIX GetMatrix() const noexcept
	{
		using namespace DirectX;

		XMVECTOR front = DirectX::XMLoadFloat3(&_front);
		XMVECTOR up = DirectX::XMLoadFloat3(&_up);
		XMVECTOR pos = DirectX::XMLoadFloat3(&_cameraPos);

		return XMMatrixLookAtLH(pos, XMVectorAdd(pos, front), up);
	}

	DirectX::XMFLOAT3 GetPos() const noexcept
	{
		return _cameraPos;
	}

	void Translate(DirectX::XMFLOAT3 translation) noexcept
	{
		namespace dx = DirectX;
		dx::XMVECTOR front = DirectX::XMLoadFloat3(&_front);
		dx::XMVECTOR up = DirectX::XMLoadFloat3(&_up);
		dx::XMVECTOR pos = DirectX::XMLoadFloat3(&_cameraPos);
		dx::XMVectorMultiply(dx::XMLoadFloat3(&translation), dx::XMLoadFloat3(&_front));
		dx::XMVECTOR sideVec = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(front, up));
		pos = dx::XMVectorAdd(pos, dx::XMVectorAdd( 
			dx::XMVectorScale(sideVec, translation.x * _cameraSpeed),
			dx::XMVectorScale(front, translation.z * _cameraSpeed)
			));

		DirectX::XMStoreFloat3(&_cameraPos, pos);
	}

	void Update(float xoffset, float yoffset)
	{
		using namespace DirectX;

		// Apply mouse sensitivity (uncomment or modify sensitivity factor as needed)
		float sensitivity = 0.05f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		// Update yaw and pitch based on the mouse movement
		_yaw += xoffset;
		_pitch += yoffset;

		// Clamp the pitch value to prevent flipping
		_pitch = std::clamp(_pitch, -89.0f, 89.0f);

		// Convert pitch and yaw to radians for trigonometric functions
		float pitchRad = DirectX::XMConvertToRadians(_pitch);
		float yawRad = DirectX::XMConvertToRadians(_yaw);

		// Update the front vector based on the yaw and pitch
		_front.x = cosf(pitchRad) * cosf(yawRad);
		_front.y = sinf(pitchRad);  
		_front.z = cosf(pitchRad) * sinf(yawRad);

		// Normalize the front vector
		auto vec = DirectX::XMLoadFloat3(&_front);
		vec = DirectX::XMVector3Normalize(vec);
		DirectX::XMStoreFloat3(&_front, vec);
	}
};