#pragma once
#include "Plane.hpp"
#include "BindableCodex.hpp"
#include "BindableBase.hpp"
#include "Drawable.hpp"

class TestPlane : public Drawable
{
private:
	DirectX::XMFLOAT3 _pos = { 0.0f, 10.0f, 3.0f };
	DirectX::XMFLOAT3 _scale = { 2.0f, 2.0f, 1.0f };

public:
	TestPlane(Graphics & gfx);
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void SpawnControl() noexcept;
};