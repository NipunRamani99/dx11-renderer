#pragma once
#include "Graphics.hpp"
#include <DirectXMath.h>
namespace Bind {
	class Bindable;
	class IndexBuffer;
}
class Drawable {
private:
	const Bind::IndexBuffer* pIndexBuffer = nullptr;
	std::vector<std::shared_ptr<Bind::Bindable>> binds;

protected:
	template<class T>
	T* QueryBindable() noexcept
	{
		for (auto& pb : binds)
		{
			if (auto pt = dynamic_cast<T*>(pb.get()))
			{
				return pt;
			}
		}
		return nullptr;
	}

public:
	Drawable() = default;
	Drawable( const Drawable& ) = delete;
	virtual DirectX::XMMATRIX GetTransformXM() const noexcept = 0;
	void Draw( Graphics& gfx ) const noexcept(!IS_DEBUG);
	void AddBind( std::shared_ptr<Bind::Bindable> bind ) noexcept(!IS_DEBUG);
	virtual ~Drawable() = default;
};