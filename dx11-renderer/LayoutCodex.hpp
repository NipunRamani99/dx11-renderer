#pragma once
#include "DynamicConstantBuffer.hpp"
#include <string>
#include <memory>
#include <unordered_map>

namespace Dcb
{
class LayoutCodex
{
  private:
    std::unordered_map<std::string, std::shared_ptr<LayoutElement>> _map;
    LayoutCodex() = default;

  public:
    static Dcb::CookedLayout Resolve( RawLayout&& layout ) noexcept
    {
        auto sig      = layout.GetSignature();
        auto& map     = Get_()._map;
        const auto it = map.find( sig );
        if( it != map.end() )
        {
            layout.ClearRoot();
            return { it->second };
        }
        auto result = map.insert( { std::move( sig ), layout.DeliverRoot() } );
        // return layout with additional reference to root
        return { result.first->second };
    }
    static LayoutCodex& Get_() noexcept
    {
        static LayoutCodex codex;
        return codex;
    }
};
}; // namespace Dcb