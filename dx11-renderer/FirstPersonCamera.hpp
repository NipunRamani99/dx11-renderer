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
	float _cameraSpeed = 1.0f;
	DirectX::XMFLOAT2 _cursorPos;
	DirectX::XMFLOAT2 _prevCursorPos;


public:
	float _pitch = 0.0f;
	float _yaw = -90.0f;
	FirstPersonCamera()
		:
		_cursorPos(float(SCREEN_WIDTH >> 1), float(SCREEN_HEIGHT >> 1)),
		_prevCursorPos(_cursorPos),
		_cameraPos(0.0f, 0.0f, -5.0f),
		_up(0.0f, 1.0f, 0.0f),
		_front(0.0f, 0.0f, 1.0f)
	{

	}

	DirectX::XMFLOAT3 GetPos()
	{
		return _cameraPos;
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

	void Update(const Keyboard& kbd, float xoffset, float yoffset)
	{
		using namespace DirectX;
		XMVECTOR front = DirectX::XMLoadFloat3(&_front);
		XMVECTOR up = DirectX::XMLoadFloat3(&_up);
		XMVECTOR pos = DirectX::XMLoadFloat3(&_cameraPos);
		XMVECTOR sideVec = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(front, up));
		
		// Define a movement direction vector and initialize it to zero
		XMVECTOR movement = XMVectorZero();

		// Handle input for strafing (left/right)
		if (kbd.KeyIsPressed('A'))
		{
			movement = XMVectorAdd(movement, sideVec);
		}
		else if (kbd.KeyIsPressed('D'))
		{
			movement = XMVectorSubtract(movement, sideVec);
		}

		// Handle input for moving forward/backward
		if (kbd.KeyIsPressed('W'))
		{
			movement = XMVectorAdd(movement, front);
		}
		else if (kbd.KeyIsPressed('S'))
		{
			movement = XMVectorSubtract(movement, front);
		}

		// Apply movement to the camera position if movement is not zero
		if (!XMVector3Equal(movement, XMVectorZero()))
		{
			movement = XMVector3Normalize(movement); // Normalize movement to ensure consistent speed
			pos = XMVectorAdd(pos, XMVectorScale(movement, _cameraSpeed));
		}

		// Update _cameraPos (store the result back)
		XMStoreFloat3(&_cameraPos, pos);

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