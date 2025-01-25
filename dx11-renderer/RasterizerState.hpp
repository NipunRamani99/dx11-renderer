#pragma once
#include "Bindable.hpp"
#include "BindableCodex.hpp"

namespace Bind
{
	class RasterizerState : public Bindable
	{
	private:
		bool _twoSided = false;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> _pRasterizerState;
	
	public:
		RasterizerState(Graphics & gfx, bool twoSided)
			:
			_twoSided(twoSided)
		{
			D3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
			rasterizerDesc.CullMode = twoSided ? D3D11_CULL_NONE : D3D11_CULL_BACK;
			GetDevice(gfx)->CreateRasterizerState(&rasterizerDesc, &_pRasterizerState);
		}

		void Bind(Graphics& gfx) noexcept
		{
			GetContext(gfx)->RSSetState(_pRasterizerState.Get());
		}

		static std::shared_ptr<RasterizerState> Resolve(Graphics& gfx, bool twoSided)
		{
			return Codex::Resolve<RasterizerState>(gfx, twoSided);
		}

		static std::string GenerateUID(bool twoSided)
		{
			return typeid(RasterizerState).name() + twoSided ? "Y" : "N";
		}
	};
};