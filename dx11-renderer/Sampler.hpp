#pragma once
#include "Bindable.hpp"

class Sampler : public Bindable {
private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
public:
	Sampler(Graphics& gfx);
	void Bind(Graphics& gfx) noexcept override;
};