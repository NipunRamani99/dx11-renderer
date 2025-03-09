#include "TestPlane.hpp"
#include "imgui\imgui.h"
#include "ConstantBufferEx.hpp"
#include "DynamicConstantBuffer.hpp"

TestPlane::TestPlane( Graphics& gfx, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 color, std::string name, float scale )
    : _name( name )
{
    auto model = Plane::Make( scale );
    AddBind( Bind::VertexBuffer::Resolve( gfx, "TestPlane", model.vertices ) );
    AddBind( Bind::IndexBuffer::Resolve( gfx, "TestPlane", model.indices ) );
    AddBind( Bind::Topology::Resolve( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );
    auto vs    = Bind::VertexShader::Resolve( gfx, "./SolidVS.cso" );
    auto pvsbc = vs->GetBytecode();
    AddBind( vs );
    AddBind( Bind::PixelShader::Resolve( gfx, "./SolidPS.cso" ) );
    AddBind( Bind::InputLayout::Resolve( gfx, model.vertices.GetVertexLayout(), pvsbc ) );
    AddBind( Bind::Texture::Resolve( gfx, "./models/brick_wall/brick_wall_diffuse.jpg", 0u ) );
    AddBind( Bind::Texture::Resolve( gfx, "./models/brick_wall/brick_wall_normal.jpg", 2u ) );
    AddBind( Bind::Sampler::Resolve( gfx, 1u ) );
    AddBind( Bind::Blender::Resolve( gfx, true, 0.5f ) );
    AddBind( Bind::RasterizerState::Resolve( gfx, true ) );  
    _pos             = pos;
    Dcb::RawLayout rawLayout;
    rawLayout.Add<Dcb::Float4>( "color" );
    buffer               = std::make_shared<Dcb::Buffer>( std::move( rawLayout ) );
    ( *buffer )["color"] = DirectX::XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f );
    _pDcb                = std::make_shared<Bind::CachingPixelConstantBufferEX>( gfx, *buffer, 0u );
    AddBind( _pDcb );
    // AddBind(Bind::PixelConstantBuffer<NormalData>::Resolve(gfx, normalData, 4u));
    AddBind( std::make_shared<Bind::TransformCbufDoubleBoi>( gfx, *this ) );
}

DirectX::XMMATRIX TestPlane::GetTransformXM() const noexcept
{
    return DirectX::XMMatrixRotationRollPitchYaw( DirectX::XMConvertToRadians( _roll ),
                                                  DirectX::XMConvertToRadians( _pitch ),
                                                  DirectX::XMConvertToRadians( _yaw ) ) *
           DirectX::XMMatrixTranslation( _pos.x, _pos.y, _pos.z );
}

void TestPlane::SpawnControl( Graphics& gfx ) noexcept
{
    ImGui::Begin( _name.c_str() );
    ImGui::Text( "Position" );
    ImGui::SameLine();
    ImGui::InputFloat3( "##PositionInput", (float*)&_pos );
    // ImGui::Text("Scale");
    // ImGui::SameLine();
    // ImGui::InputFloat3("##ScaleInput", (float*)&_scale);
    // ImGui::Text("Pitch");
    // ImGui::SameLine();
    // ImGui::InputFloat("##PitchInput", &_pitch);
    // ImGui::Text("Yaw");
    // ImGui::SameLine();
    // ImGui::InputFloat("##YawInput", &_yaw);
    // ImGui::Text("Roll");
    // ImGui::SameLine();
    // ImGui::InputFloat("##RollInput", &_roll);
    // bool changed = ImGui::SliderFloat("Spec Weight", &normalData.specularMapWeight, 0.0f, 2.0f) ||
    //	ImGui::ColorPicker3("Spec Color", reinterpret_cast<float*>(&normalData.specularColor));
    // if (changed)
    //{
    //	Bind::PixelConstantBuffer<NormalData>::Resolve(gfx, normalData, 4u)->Update(gfx, normalData);
    // }
    // changed = false;
    // changed = ImGui::SliderFloat("Spec. Int.", &objectData.specularIntensity, 0.0f, 1.0f) || ImGui::SliderFloat("Spec
    // Pow", &objectData.specularPower, 0.0f, 1000.0f, "%f"); if(changed)
    //{
    //	Bind::PixelConstantBuffer<ObjectData>::Resolve(gfx, objectData, 1u)->Update(gfx, objectData);
    // }
    //
    ImGui::Text( "Color" );
    ImGui::SameLine();
    DirectX::XMFLOAT4* color = &( *buffer )["color"];
    if( ImGui::InputFloat4( "##Color Input", (float*)color ) )
    {
        _pDcb->Update( gfx, *buffer );
    }

    ImGui::End();
}
