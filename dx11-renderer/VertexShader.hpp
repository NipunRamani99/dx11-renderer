#pragma once
#include "Bindable.hpp"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.hpp"
namespace Bind 
{
	class VertexShader : public Bindable {
	private:
		Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
		std::string _path = "";
	public:
		VertexShader(Graphics& gfx, const std::string& path);

		void Bind(Graphics& gfx) noexcept override;

		ID3DBlob* GetBytecode() const noexcept;
		
		static std::shared_ptr<VertexShader> Resolve(Graphics& gfx, const std::string& path)
		{
			return Codex::Get().Resolve<VertexShader>(gfx, path);
		}

		static std::string GenerateUID(const std::string path)
		{
			using namespace std::string_literals;
			return typeid(VertexShader).name() + "#"s + path;
		}

		std::string GetUID() const noexcept override
		{
			return GenerateUID(_path);
		}
	};
}