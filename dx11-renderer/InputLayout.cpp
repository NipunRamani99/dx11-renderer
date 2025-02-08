#include "InputLayout.hpp"

using namespace Bind;

InputLayout::InputLayout( Graphics& gfx, const Dvtx::VertexLayout& layout, ID3DBlob* pVertexShaderBytecode )
    : _layout( layout )
{
    INFOMAN( gfx );
    const std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = _layout.GetInputLayout();

    // input (vertex) layout (2d position only)
    GFX_THROW_INFO( GetDevice( gfx )->CreateInputLayout( inputElements.data(), (UINT)inputElements.size(),
                                                         pVertexShaderBytecode->GetBufferPointer(),
                                                         pVertexShaderBytecode->GetBufferSize(), &pInputLayout ) );
}

void InputLayout::Bind( Graphics& gfx ) noexcept
{
    GetContext( gfx )->IASetInputLayout( pInputLayout.Get() );
}
