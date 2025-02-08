#include "PixelShader.hpp"

using namespace Bind;

PixelShader::PixelShader ( Graphics& gfx, const std::string& path ) : _path ( path )
{
    INFOMAN ( gfx );
    Microsoft::WRL::ComPtr<ID3DBlob> blob;
    GFX_THROW_INFO ( D3DReadFileToBlob ( std::wstring{ path.begin (), path.end () }.c_str (), &blob ) );
    GFX_THROW_INFO ( GetDevice ( gfx )->CreatePixelShader ( blob->GetBufferPointer (), blob->GetBufferSize (), nullptr,
                                                            &pPixelShader ) );
}

void PixelShader::Bind ( Graphics& gfx ) noexcept
{
    GetContext ( gfx )->PSSetShader ( pPixelShader.Get (), nullptr, 0u );
}
