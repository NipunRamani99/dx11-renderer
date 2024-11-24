#pragma once
#include "Bindable.hpp"
#include "GraphicsThrowMacros.h"
#include "Vertex.h"
#include "BindableCodex.hpp"

namespace Bind 
{
	class VertexBuffer : public Bindable {
	protected:
		UINT stride;
		std::string _tag;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;

	public:
		VertexBuffer(Graphics& gfx, const std::string& tag, const Dvtx::VertexBuffer& vbuf)
			:
			stride(vbuf.GetVertexLayout().Size()),
			_tag(tag)
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

		void Bind(Graphics& gfx) noexcept override
		{
			const UINT offset = 0u;
			GetContext(gfx)->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);
		}

		static std::shared_ptr<VertexBuffer> Resolve(Graphics& gfx, const std::string& tag, const Dvtx::VertexBuffer& vbuf)
		{
			return Codex::Get().Resolve<VertexBuffer>(gfx, tag, vbuf);
		}
		
		static std::string GenerateUID(const std::string& tag, const Dvtx::VertexBuffer & vbuf)
		{
			return tag;
		}

		std::string GetUID() const noexcept override
		{
			return _tag;
		}
	};
}