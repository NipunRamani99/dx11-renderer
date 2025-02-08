#include "Sampler.hpp"
#include "GraphicsThrowMacros.h"

using namespace Bind;

Sampler::Sampler ( Graphics& gfx, const unsigned int slot ) : slot ( slot )
{
    INFOMAN ( gfx );
    D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
    samplerDesc.Filter             = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU           = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV           = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MaxAnisotropy      = 16;
    GFX_THROW_INFO ( GetDevice ( gfx )->CreateSamplerState ( &samplerDesc, &pSampler ) );
}

void Sampler::Bind ( Graphics& gfx ) noexcept
{
    GetContext ( gfx )->PSSetSamplers ( slot, 1, pSampler.GetAddressOf () );
}
