#pragma once
#include "Bindable.hpp"
#include "BindableCodex.hpp"
#include "GraphicsThrowMacros.h"
namespace Bind
{
	class PixelShader : public Bindable {
	protected:
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
		std::string _path = "";

	public:
		PixelShader(Graphics& gfx, const std::string& path);

		void Bind(Graphics& gfx) noexcept override;

		static std::shared_ptr<PixelShader> Resolve(Graphics& gfx, const std::string& path)
		{
			return Codex::Get().Resolve<PixelShader>(gfx, path);
		}

		static std::string GenerateUID(const std::string path)
		{
			using namespace std::string_literals;
			return typeid(PixelShader).name() + "#"s + path;
		}
		std::string GetUID() const noexcept override
		{
			return GenerateUID(_path);
		}
	};
}