#pragma once
#include "Bindable.hpp"
#include "GraphicsThrowMacros.h"
class VertexShader : public Bindable {
private:
	Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
public:
	VertexShader(Graphics& gfx, const std::wstring & path);

	void Bind(Graphics& gfx) noexcept override;

	ID3DBlob* GetBytecode() const noexcept;
};