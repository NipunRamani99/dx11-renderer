#pragma once
#include "Bindable.hpp"
namespace Bind
{
	class Sampler : public Bindable {
	private:
		Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
		const unsigned int slot = 0;
	public:
		Sampler(Graphics& gfx, const unsigned int slot);
		void Bind(Graphics& gfx) noexcept override;
	};
}