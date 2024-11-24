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
	private:
		struct Transforms {
			DirectX::XMMATRIX model;
			DirectX::XMMATRIX view;
			DirectX::XMMATRIX projection;
			const std::string GetId() const
			{
				return "cbuf_transform";
			}
		};

	private:
		const Drawable& parent;
		static std::unique_ptr<VertexConstantBuffer<Transforms>>  pVcbuf;


	public:
		TransformCbuf(Graphics& gfx, const Drawable& parent);

		void Bind(Graphics& gfx) noexcept override;

	};
}