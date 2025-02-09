#pragma once
#include "Bindable.hpp"
#include "BindableCodex.hpp"
#include <optional>
#include <array>
#include <string>

namespace Bind
{
class Blender : public Bindable
{
  private:
    bool _isBlending = false;
    Microsoft::WRL::ComPtr<ID3D11BlendState> _pBlendState;
    std::optional<std::array<float, 4>> _factors;

  public:
    Blender( Graphics& gfx, bool isBlending, std::optional<float> factors_in ) : _isBlending( isBlending )
    {
        if( factors_in )
        {
            _factors.emplace();
            _factors->fill( factors_in.value() );
        }

        D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
        auto& rtBlendDesc          = blendDesc.RenderTarget[0];
        if( isBlending )
        {
            rtBlendDesc.BlendEnable           = TRUE;
            rtBlendDesc.BlendOp               = D3D11_BLEND_OP_ADD;
            rtBlendDesc.SrcBlendAlpha         = D3D11_BLEND_ZERO;
            rtBlendDesc.DestBlendAlpha        = D3D11_BLEND_ZERO;
            rtBlendDesc.BlendOpAlpha          = D3D11_BLEND_OP_ADD;
            rtBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

            if( _factors )
            {
                rtBlendDesc.SrcBlend  = D3D11_BLEND_BLEND_FACTOR;
                rtBlendDesc.DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
            }
            else
            {
                rtBlendDesc.SrcBlend  = D3D11_BLEND_SRC_ALPHA;
                rtBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            }
        }
        else
            rtBlendDesc.BlendEnable = FALSE;
        GetDevice( gfx )->CreateBlendState( &blendDesc, &_pBlendState );
    }

    void Bind( Graphics& gfx ) noexcept
    {
        const float* data = _factors ? _factors->data() : nullptr;
        GetContext( gfx )->OMSetBlendState( _pBlendState.Get(), data, 0xFFFFFFFFu );
    }

    static std::shared_ptr<Blender> Resolve( Graphics& gfx, bool isBlending, std::optional<float> factors_in = {} )
    {
        return Codex::Resolve<Blender>( gfx, isBlending, factors_in );
    }

    static std::string GenerateUID( bool isBlending, std::optional<float> factors_in )
    {
        using namespace std::string_literals;
        return typeid( Blender ).name() + ( isBlending ? "ON"s : "OFF"s ) +
               ( factors_in ? "#f"s + std::to_string( *factors_in ) : "" );
    }

    void SetFactors( float factor )
    {
        if( _factors )
        {
            _factors->fill( factor );
        }
    }

    float GetFactor()
    {
        if( _factors )
        {
            return _factors->front();
        }
        else
        {
            return -1.0f;
        }
    }

    std::string GetUID() const noexcept override
    {
        return GenerateUID( _isBlending, _factors ? _factors->front() : std::optional<float>{} );
    }
};
} // namespace Bind
