#pragma once
#include "Bindable.hpp"
using namespace Bind;
ID3D11DeviceContext* Bindable::GetContext( Graphics& gfx ) noexcept
{
    return gfx.pContext.Get();
}

ID3D11Device* Bindable::GetDevice( Graphics& gfx ) noexcept
{
    return gfx.pDevice.Get();
}

DxgiInfoManager& Bindable::GetInfoManager( Graphics& gfx ) noexcept( IS_DEBUG )
{
#ifndef NDEBUG
    return gfx.infoManager;
#else
    throw std::logic_error( "Tried to access gfx.infoManager while in Release mode!" );
#endif
}
