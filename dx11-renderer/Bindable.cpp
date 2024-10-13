#pragma once
#include "Bindable.hpp"

ID3D11DeviceContext* Bind::Bindable::GetContext(Graphics& gfx) noexcept
{
	return gfx.pContext.Get();
}

ID3D11Device* Bind::Bindable::GetDevice(Graphics& gfx) noexcept
{
	return gfx.pDevice.Get();
}

DxgiInfoManager& Bind::Bindable::GetInfoManager(Graphics& gfx) noexcept(IS_DEBUG)
{
#ifndef NDEBUG
	return gfx.infoManager;
#else
	throw std::logic_error("YouFuckedUp! (tried to access gfx.infoManager while in Release mode!)");
#endif
}
