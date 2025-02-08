#pragma once
#include "Bindable.hpp"
#include "BindableCodex.hpp"

namespace Bind
{
class Texture : public Bindable
{
  private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _pTextureView;
    const unsigned int _slot = 0;
    std::string _path        = "";
    bool hasAlpha            = false;

  public:
    Texture ( Graphics& gfx, const std::string& path, const unsigned int slot );

    void Bind ( Graphics& gfx ) noexcept override;

    static std::shared_ptr<Texture> Resolve ( Graphics& gfx, const std::string& path, const unsigned int slot )
    {
        return Codex::Get ().Resolve<Texture> ( gfx, path, slot );
    }

    static const std::string GenerateUID ( const std::string& path, const unsigned int slot )
    {
        using namespace std::string_literals;
        return path + "#"s + std::to_string ( slot );
    }

    std::string GetUID () const noexcept override
    {
        using namespace std::string_literals;
        return _path + "#"s + std::to_string ( _slot );
    }

    const bool HasAlpha () const noexcept
    {
        return hasAlpha;
    }
};
} // namespace Bind
