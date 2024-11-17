#pragma once
#include "Bindable.hpp"
class Surface;
namespace Bind
{
	class Texture : public Bindable {
	private:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _pTextureView;
		const unsigned int _slot = 0;
		std::string _tag = "";
	public:
		Texture(Graphics& gfx, const class Surface& s, const std::string & tag, const unsigned int slot);
		void Bind(Graphics& gfx) noexcept override;
		static const std::string GenerateUID(const class Surfac& s, const std::string& tag, const unsigned int slot)
		{
			return tag;
		}
		std::string GetUID() const noexcept override
		{
			return _tag;
		}
	};
}