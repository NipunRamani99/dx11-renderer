#pragma once
#include "Drawable.hpp"

class SolidSphere : public Drawable
{
  public:
    SolidSphere( Graphics& gfx, float radius );
    void SetPos( DirectX::XMFLOAT3 pos ) noexcept;
    DirectX::XMMATRIX GetTransformXM() const noexcept override;

  private:
    // positional
    DirectX::XMFLOAT3 _pos = { 1.0f, 1.0f, 1.0f };
};
