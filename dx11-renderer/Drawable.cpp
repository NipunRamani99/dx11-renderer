#pragma once
#include "Drawable.hpp"
#include "GraphicsThrowMacros.h"
#include "IndexBuffer.hpp"
#include <cassert>
#include <typeinfo>
using namespace Bind;
void Drawable::Draw( Graphics& gfx ) const noexcept( !IS_DEBUG )
{

    for ( auto& b : binds )
    {
        b->Bind( gfx );
    }
    UINT count = pIndexBuffer->GetCount();
    gfx.DrawIndexed( count );
}

void Drawable::AddBind( std::shared_ptr<Bindable> bind ) noexcept( !IS_DEBUG )
{
    if ( typeid( *bind ) == typeid( IndexBuffer ) )
    {
        assert( "Cannot bind IndexBuffer more than one time" && pIndexBuffer == nullptr );
        pIndexBuffer = (IndexBuffer*)bind.get();
    }
    binds.push_back( bind );
}
