#pragma once
#include "Graphics.hpp"
#include "SolidSphere.hpp"
#include "ConstantBuffers.hpp"

class PointLight {
private:
	struct PointLightCBuf
	{
		alignas(16) mutable DirectX::XMFLOAT3 pos;
		alignas(16) mutable DirectX::XMFLOAT3 ambient;
		alignas(16) DirectX::XMFLOAT3 diffuseColor;
		alignas(16) mutable DirectX::XMFLOAT3 lightDirection;
		float diffuseIntensity = 1.0f;
		float attConst = 1.0f;
		float attLin = 0.045f;
		float attQuad = 0.0075f;
	};

	mutable Bind::PixelConstantBuffer<PointLightCBuf> cbuf;
	mutable SolidSphere mesh;
	DirectX::XMFLOAT3 _pos = { 0.0f,0.0f,0.0f };
	PointLightCBuf cbdata;
public:
	PointLight(Graphics& gfx, float radius = 0.5f);
	void SpawnControlWindow() noexcept;
	void Reset() noexcept;
	void Draw(Graphics& gfx) const noexcept(!IS_DEBUG);
	void Bind(Graphics& gfx, DirectX::FXMMATRIX& view) const noexcept;
};