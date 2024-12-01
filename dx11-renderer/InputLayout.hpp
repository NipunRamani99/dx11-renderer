#pragma once
#include "Bindable.hpp"
#include "BindableCodex.hpp"
#include "GraphicsThrowMacros.h"
#include "Vertex.h"
#include <string>
namespace Bind
{
	class InputLayout : public Bindable {
	protected:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
		Dvtx::VertexLayout _layout;

	public:
		InputLayout(Graphics& gfx,
			const Dvtx::VertexLayout & layout,
			ID3DBlob* pVertexShaderBytecode);

		void Bind(Graphics& gfx) noexcept override;

		static std::shared_ptr<Bindable> Resolve(Graphics& gfx, const Dvtx::VertexLayout& layout, ID3DBlob* pVertexShaderByteCode)
		{
			return Codex::Get().Resolve<InputLayout>(gfx, layout, pVertexShaderByteCode);
		}

		template<typename...Ignore>
		static std::string GenerateUID(const Dvtx::VertexLayout& layout, Ignore&&... ignore)
		{
			using namespace std::string_literals;
			std::string name =  typeid(InputLayout).name() + "#"s + layout.GetCode();
			return name;
		}

		std::string GetUID() const noexcept override
		{
			using namespace std::string_literals;
			return typeid(InputLayout).name() + "#"s + _layout.GetCode();
		}
	};
}