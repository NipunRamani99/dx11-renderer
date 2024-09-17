#pragma once
#include "DrawableBase.hpp"
#include <DirectXMath.h>

class AssimpTest : public DrawableBase<AssimpTest> 
{
private:
	// positional
	float r;
	float roll = 0.0f;
	float pitch = 0.0f;
	float yaw = 0.0f;
	float theta;
	float phi;
	float chi;
	// speed (delta/s)
	float droll;
	float dpitch;
	float dyaw;
	float dtheta;
	float dphi;
	float dchi;
	// model transform
	DirectX::XMFLOAT3X3 mt;
	DirectX::XMFLOAT3 material;

	float scale;

public:
	AssimpTest(Graphics& gfx, std::mt19937& rng,
		std::uniform_real_distribution<float>& adist,
		std::uniform_real_distribution<float>& ddist,
		std::uniform_real_distribution<float>& odist,
		std::uniform_real_distribution<float>& rdist,
		DirectX::XMFLOAT3 material,
		float scale);
	
	void Update(float dt) noexcept override;
	
	DirectX::XMMATRIX GetTransformXM() const noexcept override;

	void BindForPhongShader(Graphics& gfx, float scale);
};