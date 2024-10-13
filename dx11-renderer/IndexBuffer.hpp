#pragma once
#include "Bindable.hpp"
#include "GraphicsThrowMacros.h"
#include "Graphics.hpp"
namespace Bind
{
	class IndexBuffer : public Bindable {
	protected:
		UINT count;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
	public:
		IndexBuffer(Graphics& gfx, const std::vector<unsigned int>& indices);
		void Bind(Graphics& gfx) noexcept override;
		UINT GetCount() const noexcept;
	};
}