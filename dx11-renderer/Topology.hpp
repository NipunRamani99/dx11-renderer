#pragma once
#include "Bindable.hpp"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.hpp"
namespace Bind
{
class Topology : public Bindable
{
  protected:
    D3D11_PRIMITIVE_TOPOLOGY type;

  public:
    Topology( Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type );

    void Bind( Graphics& gfx ) noexcept override;

    static std::shared_ptr<Topology> Resolve( Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type )
    {
        return Codex::Get().Resolve<Topology>( gfx, type );
    }

    static std::string GenerateUID( D3D11_PRIMITIVE_TOPOLOGY type )
    {
        using namespace std::string_literals;
        return typeid( Topology ).name() + "#"s + std::to_string( (unsigned int)( type ) );
    }

    std::string GetUID() const noexcept override
    {
        return GenerateUID( type );
    }
};
} // namespace Bind
