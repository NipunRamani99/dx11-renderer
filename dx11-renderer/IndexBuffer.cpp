#include "IndexBuffer.hpp"

using namespace Bind;

IndexBuffer::IndexBuffer(Graphics& gfx, const std::string& tag, const std::vector<unsigned int>& indices)
	:
	count((UINT)indices.size()),
	_tag(tag)
{
	INFOMAN(gfx);
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.CPUAccessFlags = 0u;
	bufferDesc.MiscFlags = 0u;
	bufferDesc.ByteWidth = UINT(sizeof(unsigned int) * count);
	bufferDesc.StructureByteStride = sizeof(unsigned int);
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = indices.data();
	GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bufferDesc, &sd, &pIndexBuffer));
}

void IndexBuffer::Bind(Graphics& gfx) noexcept
{
	GetContext(gfx)->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
}

UINT IndexBuffer::GetCount() const noexcept
{
	return count;
}
