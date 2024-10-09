#pragma once
#include "DrawableBase.hpp"

class SolidSphere : public DrawableBase<SolidSphere>
{
public:
	SolidSphere(Graphics& gfx, float radius);
	void Update(float dt) noexcept override;
	void SetPos(DirectX::XMFLOAT3 pos) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
private:
	// positional
	DirectX::XMFLOAT3 _pos = { 1.0f,1.0f,1.0f };
};