#pragma once
#pragma warning( disable : 4100 )
#include "Bindable.hpp"
#include <type_traits>
#include <unordered_map>
#include <memory>
namespace Bind
{
class Codex
{
  private:
    std::unordered_map<std::string, std::shared_ptr<Bindable>> _binds;

  public:
    static Codex& Get ()
    {
        static Codex codex;
        return codex;
    }
    template <typename T, typename... Args> static std::shared_ptr<T> Resolve ( Graphics& gfx, Args&&... args )
    {
        static_assert ( std::is_base_of<Bindable, T>::value, "Can Resolve only derived types from Bindable" );
        return Get ()._Resolve<T> ( gfx, std::forward<Args> ( args )... );
    }

  private:
    template <typename T, typename... Args> std::shared_ptr<T> _Resolve ( Graphics& gfx, Args&&... args )
    {

        std::string key = T::GenerateUID ( std::forward<Args> ( args )... );
        if ( _binds.find ( key ) == _binds.end () )
        {
            std::shared_ptr<T> bindable = std::make_shared<T> ( gfx, std::forward<Args> ( args )... );
            _binds[key]                 = std::dynamic_pointer_cast<Bindable> ( bindable );
            return bindable;
        }
        else
        {
            return std::dynamic_pointer_cast<T> ( _binds[key] );
        }
    }
};
} // namespace Bind
