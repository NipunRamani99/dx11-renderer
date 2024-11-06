#pragma once
#include "Bindable.hpp"
class Surface;
namespace Bind
{
	class Texture : public Bindable {
	private:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
		const unsigned int slot = 0;
	public:
		Texture(Graphics& gfx, const class Surface& s, const unsigned int slot);
		void Bind(Graphics& gfx) noexcept override;
	};
}