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

    struct NormalData
    {
        alignas( 16 ) BOOL hasNormalMap = TRUE;
        BOOL hasSpecularMap             = FALSE;
        BOOL negateYAndZ                = FALSE;
        BOOL hasGloss                   = FALSE;
        DirectX::XMFLOAT3 specularColor = { 0.75f, 0.75f, 0.75f };
        float specularMapWeight         = 1.0f;
        static std::string GetId()
        {
            return "TestPlaneNormalData";
        }
    } normalData;

    struct ObjectData
    {
        /*alignas(16) DirectX::XMFLOAT3 material;
        float specularIntensity = 0.60f;
        float specularPower = 30.0f;
        float padding[1];
        std::string name;*/
        DirectX::XMFLOAT4 color;
        static std::string GetId()
        {
            return "ObjectData";
        }
    } objectData;

    std::shared_ptr < Bind::CachingPixelConstantBufferEX > _pDcb;
    std::string _name = "Test Plane";
    std::shared_ptr<Dcb::Buffer> buffer; 
  public:
    TestPlane( Graphics& gfx, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 color, std::string name, float scale = 1.0f );
    DirectX::XMMATRIX GetTransformXM() const noexcept override;
    void SpawnControl( Graphics& gfx ) noexcept;
};
