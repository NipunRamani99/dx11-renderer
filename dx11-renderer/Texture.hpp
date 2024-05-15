#pragma once
#include "Bindable.hpp"

class Texture : public Bindable {
private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
public:
	Texture(Graphics& gfx, const class Surface& s);
	void Bind(Graphics& gfx) noexcept override;
};