#include "Texture.hpp"
#include "Surface.hpp"
#include "GraphicsThrowMacros.h"

namespace wrl = Microsoft::WRL;

using namespace Bind;

Texture::Texture ( Graphics& gfx, const std::string& path, const unsigned int slot ) : _slot ( slot ), _path ( path )
{
    INFOMAN ( gfx );

    // Load surface from file
    const Surface s = Surface::FromFile ( path );
    hasAlpha        = s.HasAlpha ();

    // create texture resource
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width                = s.GetWidth ();
    textureDesc.Height               = s.GetHeight ();
    textureDesc.MipLevels            = 0;
    textureDesc.ArraySize            = 1;
    textureDesc.Format               = DXGI_FORMAT_B8G8R8A8_UNORM;
    textureDesc.SampleDesc.Count     = 1;
    textureDesc.SampleDesc.Quality   = 0;
    textureDesc.Usage                = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    textureDesc.CPUAccessFlags       = 0;
    textureDesc.MiscFlags            = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    D3D11_SUBRESOURCE_DATA sd        = {};
    sd.pSysMem                       = s.GetBufferPtr ();
    sd.SysMemPitch                   = s.GetWidth () * sizeof ( Surface::Color );
    wrl::ComPtr<ID3D11Texture2D> pTexture;
    GFX_THROW_INFO ( GetDevice ( gfx )->CreateTexture2D ( &textureDesc, nullptr, &pTexture ) );

    GetContext ( gfx )->UpdateSubresource ( pTexture.Get (), 0, 0, s.GetBufferPtr (), sd.SysMemPitch, 0 );

    // create the resource view on the texture
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                          = textureDesc.Format;
    srvDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip       = 0;
    srvDesc.Texture2D.MipLevels             = -1;
    GFX_THROW_INFO ( GetDevice ( gfx )->CreateShaderResourceView ( pTexture.Get (), &srvDesc, &_pTextureView ) );

    GetContext ( gfx )->GenerateMips ( _pTextureView.Get () );
}

void Texture::Bind ( Graphics& gfx ) noexcept
{
    GetContext ( gfx )->PSSetShaderResources ( _slot, 1u, _pTextureView.GetAddressOf () );
}
