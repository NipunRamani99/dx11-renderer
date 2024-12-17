#include "Sampler.hpp"
#include "GraphicsThrowMacros.h"

using namespace Bind;

Sampler::Sampler(Graphics& gfx, const unsigned int slot)
	:
	slot(slot)
{
	INFOMAN(gfx);
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	GFX_THROW_INFO(GetDevice(gfx)->CreateSamplerState(&samplerDesc, &pSampler));
}

void Sampler::Bind(Graphics& gfx) noexcept
{
	GetContext(gfx)->PSSetSamplers(slot, 1, pSampler.GetAddressOf());
}
