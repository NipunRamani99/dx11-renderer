#pragma once
#include "Bindable.hpp"
#include "GraphicsThrowMacros.h"
namespace Bind
{
	template<typename C>
	class ConstantBuffer : public Bindable {
	protected:
		Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
		UINT slot;
	public:
		ConstantBuffer(Graphics& gfx, const C& consts, UINT _slot = 0)
			:
			slot(_slot)
		{

			INFOMAN(gfx);
			D3D11_BUFFER_DESC cbd;
			cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbd.Usage = D3D11_USAGE_DYNAMIC;
			cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbd.MiscFlags = 0u;
			cbd.ByteWidth = sizeof(C);
			cbd.StructureByteStride = 0u;
			D3D11_SUBRESOURCE_DATA csd = {};
			csd.pSysMem = &consts;
			GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&cbd, &csd, &pConstantBuffer));
		}
		void Update(Graphics& gfx, const C& consts) {
			INFOMAN(gfx);
			D3D11_MAPPED_SUBRESOURCE msr;
			GFX_THROW_INFO(
				GetContext(gfx)->Map(
					pConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr
				)
			);
			memcpy(msr.pData, &consts, sizeof(consts));
			GetContext(gfx)->Unmap(pConstantBuffer.Get(), 0u);
		}
		ConstantBuffer(Graphics& gfx, UINT _slot = 0)
			:
			slot(_slot)
		{
			INFOMAN(gfx);
			D3D11_BUFFER_DESC cbd;
			cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbd.Usage = D3D11_USAGE_DYNAMIC;
			cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbd.MiscFlags = 0u;
			cbd.ByteWidth = sizeof(C);
			cbd.StructureByteStride = 0u;
			GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&cbd, nullptr, &pConstantBuffer));
		}

	
	};

	template<typename C>
	class VertexConstantBuffer : public ConstantBuffer<C> {
		using ConstantBuffer<C>::pConstantBuffer;
		using ConstantBuffer<C>::slot;
		using Bindable::GetContext;
	public:
		using ConstantBuffer<C>::ConstantBuffer;
		void Bind(Graphics& gfx) noexcept override {
			GetContext(gfx)->VSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
		}

		static std::string GenerateUID(const C& consts, UINT slot = 0)
		{
			return typeid(VertexConstantBuffer).name() + "#" + consts.GetId() + "#" + std::to_string(slot);
		}

		static std::string GenerateUID(UINT slot = 0)
		{
			using namespace std::string_literals;
			return typeid(VertexConstantBuffer).name() + "#"s + std::to_string(slot);
		}
	};


	template<typename C>
	class PixelConstantBuffer : public ConstantBuffer<C> {
		using ConstantBuffer<C>::pConstantBuffer;
		using ConstantBuffer<C>::slot;
		using Bindable::GetContext;
	public:
		using ConstantBuffer<C>::ConstantBuffer;
		void Bind(Graphics& gfx) noexcept override {
			GetContext(gfx)->PSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
		}

		static std::string GenerateUID(const C& consts, UINT slot = 0)
		{
			using namespace std::string_literals;
			return typeid(PixelConstantBuffer).name() + "#"s + consts.GetId() + "#"s + std::to_string(slot);
		}

		static std::string GenerateUID(UINT slot = 0)
		{
			using namespace std::string_literals;
			return typeid(PixelConstantBuffer).name() + "#"s + std::to_string(slot);
		}
	};
}