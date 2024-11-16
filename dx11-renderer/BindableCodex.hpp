#pragma once
#include "BindableBase.hpp"
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
		static Codex& Get()
		{
			static Codex codex;
			return codex;
		}
		template<typename T, typename...Args>
		std::shared_ptr<T> Resolve(Graphics & gfx, Args...args)
		{
			assert(std::is_base_of_v<Bindable, T>::value, "Can Resolve only derived types from Bindable");
			return _Resolve<T>(gfx, std::forward<Args>(args)...);
		}
	private:
		template<typename T, typename...Args>
		std::shared_ptr<T> _Resolve(Graphics & gfx, Args...args)
		{

			std::string key = T::GenerateUID(std::forward<Args>(args));
			if (_binds.find(key) == _binds.end())
			{
				std::shared_ptr<T> bindable = std::make_shared<T>(gfx, std::forward<Args>(args)...);
				_binds[key] = std::dynamic_pointer_cast<Base>(bindable);
				return bindable;
 			}
			else
			{
				return std::dynamic_pointer_cast<T>(_binds[key]);
			}
		}
	};
}