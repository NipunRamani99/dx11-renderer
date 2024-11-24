#pragma once
#include "Bindable.hpp"
#include "GraphicsThrowMacros.h"
#include "Graphics.hpp"
#include "BindableCodex.hpp"
namespace Bind
{
	class IndexBuffer : public Bindable {
	protected:
		UINT count;
		std::string _tag = "";

		Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
	public:
		IndexBuffer(Graphics& gfx, const std::string& tag, const std::vector<unsigned int>& indices);

		void Bind(Graphics& gfx) noexcept override;

		UINT GetCount() const noexcept;
		
		static std::shared_ptr<IndexBuffer> Resolve(Graphics& gfx, const std::string& tag, const std::vector<unsigned int>& indices)
		{
			return Codex::Get().Resolve<IndexBuffer>(gfx, tag, indices);
		}

		template<typename...Ignore>
		static std::string GenerateUID(const std::string& tag, Ignore&&...ignore)
		{
			using namespace std::string_literals;
			return typeid(IndexBuffer).name() + "#"s + tag;
		}

		std::string GetUID() const noexcept override 
		{
			using namespace std::string_literals;
			return typeid(IndexBuffer).name() + "#"s + _tag;
		}
	};
}