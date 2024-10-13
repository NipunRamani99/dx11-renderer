#pragma once
#include "Graphics.hpp"
#include <DirectXMath.h>
namespace Bind {
	class Bindable;
	class IndexBuffer;
}
class Drawable {
	template<class T>
	friend class DrawableBase;
private:
	const Bind::IndexBuffer* pIndexBuffer = nullptr;
	std::vector<std::unique_ptr<Bind::Bindable>> binds;

private:
	virtual const std::vector<std::unique_ptr<Bind::Bindable>>& GetStaticBinds() const noexcept { return {}; };

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
	virtual void Update( float dt ) noexcept = 0;
	void AddBind( std::unique_ptr<Bind::Bindable> bind ) noexcept(!IS_DEBUG);
	void AddIndexBuffer( std::unique_ptr<Bind::IndexBuffer> ibuf ) noexcept(!IS_DEBUG);
	virtual ~Drawable() = default;
};