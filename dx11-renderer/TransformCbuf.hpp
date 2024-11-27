#pragma once
#include "Drawable.hpp"
#include "Bindable.hpp"
#include <DirectXMath.h>
#include <memory>
#include "ConstantBuffers.hpp"

namespace Bind
{
	class TransformCbuf : public Bindable
	{
	protected:
		struct Transforms {
			DirectX::XMMATRIX model;
			DirectX::XMMATRIX view;
			DirectX::XMMATRIX projection;
			const std::string GetId() const
			{
				return "cbuf_transform";
			}
		};
		const Drawable& parent;

	private:
		static std::unique_ptr<VertexConstantBuffer<Transforms>>  pVcbuf;
		void UpdateAndBindImpl(Graphics& gfx) noexcept;

	public:
		TransformCbuf(Graphics& gfx, const Drawable& parent);
		void Bind(Graphics& gfx) noexcept override;
	};
}