#pragma once
#include "ConstantBuffers.hpp"
#include "Drawable.hpp"
#include <DirectXMath.h>

class TransformCbuf : public Bindable
{
private:
	struct Transforms {
		DirectX::XMMATRIX modelViewProj;
		DirectX::XMMATRIX model;
	};
private:
	static std::unique_ptr<VertexConstantBuffer<Transforms>> pVcbuf;
	const Drawable& parent;
public:
	TransformCbuf(Graphics& gfx, const Drawable& parent);
	void Bind(Graphics& gfx) noexcept override;

};