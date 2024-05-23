#pragma once
#include "Graphics.hpp"
#include "SolidSphere.hpp"
#include "ConstantBuffers.hpp"

class PointLight {
private:
	struct PointLightCBuf
	{
		DirectX::XMFLOAT3 pos;
		float padding;
	};
	mutable PixelConstantBuffer<PointLightCBuf> cbuf;
	mutable SolidSphere mesh;
	DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
public:
	PointLight(Graphics& gfx, float radius = 0.5f);
	void SpawnControlWindow() noexcept;
	void Reset() noexcept;
	void Draw(Graphics& gfx) const noexcept(!IS_DEBUG);
	void Bind(Graphics& gfx) const noexcept;
};