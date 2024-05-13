#pragma once
#include "Drawable.hpp"

class Box : public Drawable {
private:

public:
	Box(Graphics& gfx);
	void Update(float dt) noexcept override;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
};