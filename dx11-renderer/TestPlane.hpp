#pragma once
#include "Plane.hpp"
#include "BindableCodex.hpp"
#include "BindableBase.hpp"
#include "Drawable.hpp"
#include "ConstantBufferEx.hpp"

class TestPlane : public Drawable
{
  private:
    DirectX::XMFLOAT3 _pos   = { 15.0f, 10.0f, -7.0f };
    DirectX::XMFLOAT3 _scale = { 2.0f, 2.0f, 1.0f };
    float _pitch             = 0.0f;
    float _roll              = 0.0f;
    float _yaw               = 0.0f;

    std::shared_ptr < Bind::CachingPixelConstantBufferEX > _pDcb;
    std::string _name = "Test Plane";
    std::shared_ptr<Dcb::Buffer> buffer; 
  public:
    TestPlane( Graphics& gfx, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 color, std::string name, float scale = 1.0f );
    DirectX::XMMATRIX GetTransformXM() const noexcept override;
    void SpawnControl( Graphics& gfx ) noexcept;
};
