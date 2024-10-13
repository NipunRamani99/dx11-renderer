#include "VertexBuffer.hpp"

using namespace Bind;

VertexBuffer::VertexBuffer(Graphics& gfx, const Dvtx::VertexBuffer& vbuf)
	:
	stride(UINT(vbuf.GetVertexLayout().Size()))
{
	INFOMAN(gfx);

	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = UINT(vbuf.Size() * vbuf.GetVertexLayout().Size());
	bd.StructureByteStride = sizeof(vbuf.GetVertexLayout().Size());

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vbuf.GetData();
	GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bd, &sd, &pVertexBuffer));
}

void VertexBuffer::Bind(Graphics& gfx) noexcept
{
	const UINT offset = 0u;
	GetContext(gfx)->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);
}
