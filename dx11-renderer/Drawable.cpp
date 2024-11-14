#pragma once
#include "Drawable.hpp"
#include "GraphicsThrowMacros.h"
#include "IndexBuffer.hpp"
#include <cassert>
#include <typeinfo>
using namespace Bind;
void Drawable::Draw(Graphics& gfx) const noexcept(!IS_DEBUG)
{

	for ( auto& b : binds )
	{
		if(b)
			b->Bind(gfx);
	}
	const std::vector<std::unique_ptr<Bindable>>& staticBinds = GetStaticBinds(); //For Box -> DrawableBase<Box>::staticBinds, For Sphere-> DrawableBase<Sphere>::staticBinds
	for (auto& b : staticBinds) {
		b->Bind(gfx);
	}
	UINT count = pIndexBuffer->GetCount();
	gfx.DrawIndexed( count );
}

void Drawable::AddBind(std::unique_ptr<Bindable> bind) noexcept(!IS_DEBUG)
{
	assert("*Must* use AddIndexBuffer to bind index buffer" && typeid(*bind) != typeid(IndexBuffer));
	binds.push_back(std::move(bind));
}

void Drawable::AddIndexBuffer(std::unique_ptr<class IndexBuffer> ibuf) noexcept(!IS_DEBUG)
{
	assert("Attempting to add index buffer a second time" && pIndexBuffer == nullptr);
	pIndexBuffer = ibuf.get();
	binds.push_back(std::move(ibuf));
}
