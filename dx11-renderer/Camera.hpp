#pragma once
#include "Graphics.hpp"
#include "ConstantBuffers.hpp"
#include <memory>
class Camera
{
private:
	float r = 20.0f;
	float phi = 0.0f;
	float theta = 0.0f;
	float pitch = 0.0f;
	float yaw = 0.0f;
	float roll = 0.0f;
	
public:
	DirectX::XMMATRIX GetMatrix() const noexcept;
	DirectX::XMFLOAT3 GetPos() const noexcept;
	void SpawnControl() noexcept;
	void Reset() noexcept;
};
