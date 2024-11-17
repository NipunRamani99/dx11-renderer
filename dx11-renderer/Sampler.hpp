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
		static std::string GenerateUID(const unsigned int slot)
		{
			using namespace std::string_literals;
			return typeid(Sampler).name() + "#"s + std::to_string(slot);
		}

		std::string GetUID() const noexcept override
		{
			return GenerateUID(slot);
		}
	};
}