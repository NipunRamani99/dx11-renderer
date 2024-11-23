#include "InputLayout.hpp"

using namespace Bind;

InputLayout::InputLayout(Graphics& gfx, const Dvtx::VertexLayout& layout, ID3DBlob* pVertexShaderBytecode)
	:
	_layout(layout)
{
	INFOMAN(gfx);
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = _layout.GetInputLayout();
	const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
	{
		{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0  }
	};
	// input (vertex) layout (2d position only)
	GFX_THROW_INFO(GetDevice(gfx)->CreateInputLayout(
		ied.data(), (UINT)std::size(ied),
		pVertexShaderBytecode->GetBufferPointer(),
		pVertexShaderBytecode->GetBufferSize(),
		&pInputLayout
	));
}

void InputLayout::Bind(Graphics& gfx) noexcept
{
	GetContext(gfx)->IASetInputLayout(pInputLayout.Get());
}
