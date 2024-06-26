#pragma once
#include "Graphics.hpp"
#include <DirectXMath.h>

class Bindable;
class IndexBuffer;
class Drawable {
	template<class T>
	friend class DrawableBase;
private:
	const IndexBuffer* pIndexBuffer = nullptr;
	std::vector<std::unique_ptr<Bindable>> binds;

private:

	virtual const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds() const noexcept { return {}; };
public:
	Drawable() = default;
	Drawable( const Drawable& ) = delete;
	virtual DirectX::XMMATRIX GetTransformXM() const noexcept = 0;
	void Draw( Graphics& gfx ) const noexcept(!IS_DEBUG);
	virtual void Update( float dt ) noexcept = 0;
	void AddBind( std::unique_ptr<Bindable> bind ) noexcept(!IS_DEBUG);
	void AddIndexBuffer( std::unique_ptr<class IndexBuffer> ibuf ) noexcept(!IS_DEBUG);
	virtual ~Drawable() = default;
};