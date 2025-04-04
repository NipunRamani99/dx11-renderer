#include "PointLight.hpp"
#include "imgui/imgui.h"

PointLight::PointLight( Graphics& gfx, float radius ) : mesh( gfx, radius ), cbuf( gfx, 0 )
{
    cbdata = { _pos, { 0.05f, 0.05f, 0.09f }, { 1.0f, 1.0f, 1.0f }, 1.0f, 1.0f, 0.045f, 0.0075f, FALSE };
}

void PointLight::SpawnControlWindow() noexcept
{
    if( ImGui::Begin( "Light" ) )
    {
        ImGui::Text( "Position" );
        ImGui::SliderFloat( "X", &_pos.x, -60.0f, 60.0f, "%.1f" );
        ImGui::SliderFloat( "Y", &_pos.y, -60.0f, 60.0f, "%.1f" );
        ImGui::SliderFloat( "Z", &_pos.z, -60.0f, 60.0f, "%.1f" );

        ImGui::Text( "Ambient Color" );
        ImGui::SliderFloat( "R#2", &cbdata.ambient.x, 0.0f, 1.0f, "%.1f" );
        ImGui::SliderFloat( "G#2", &cbdata.ambient.y, 0.0f, 1.0f, "%.1f" );
        ImGui::SliderFloat( "B#2", &cbdata.ambient.z, 0.0f, 1.0f, "%.1f" );

        ImGui::Text( "Diffuse Color" );
        ImGui::SliderFloat( "R#3", &cbdata.diffuseColor.x, 0.0f, 1.0f, "%.1f" );
        ImGui::SliderFloat( "G#3", &cbdata.diffuseColor.y, 0.0f, 1.0f, "%.1f" );
        ImGui::SliderFloat( "B#3", &cbdata.diffuseColor.z, 0.0f, 1.0f, "%.1f" );

        ImGui::Text( "Diffuse Intensity" );
        ImGui::SliderFloat( "Intensity#1", &cbdata.diffuseIntensity, 0.0f, 1.0f, "%.2f" );
        ImGui::Text( "Attenuation Constant" );
        ImGui::SliderFloat( "Intensity#2", &cbdata.attConst, 0.05f, 10.0f, "%.2f" );
        ImGui::Text( "Linear Attenuation Constant" );
        ImGui::SliderFloat( "Intensity#3", &cbdata.attLin, 0.0001f, 1.0f, "%.4f" );
        ImGui::Text( "Quad Attenuation Constant" );
        ImGui::SliderFloat( "Intensity#4", &cbdata.attQuad, 0.000001f, 1.0f, "%.7f" );
        bool renderNormal = cbdata.renderNormals == TRUE;
        ImGui::Checkbox( "Render Normals", &renderNormal );
        cbdata.renderNormals = renderNormal ? TRUE : FALSE;

        if( ImGui::Button( "Reset" ) )
        {
            Reset();
        }
    }
    ImGui::End();
}

void PointLight::Reset() noexcept
{
    _pos   = { 0.0f, 0.0f, 0.0f };
    cbdata = { _pos, { 0.05f, 0.05f, 0.09f }, { 1.0f, 1.0f, 1.0f }, 1.0f, 1.0f, 0.045f, 0.0075f, FALSE };
}

void PointLight::Draw( Graphics& gfx ) const noexcept( !IS_DEBUG )
{
    cbdata.pos = _pos;
    mesh.SetPos( _pos );
    mesh.Draw( gfx );
}

void PointLight::Bind( Graphics& gfx, DirectX::FXMMATRIX& view ) const noexcept
{
    auto dataCopy  = cbdata;
    const auto pos = DirectX::XMLoadFloat3( &cbdata.pos );
    DirectX::XMStoreFloat3( &dataCopy.pos, DirectX::XMVector3Transform( pos, view ) );
    cbuf.Update( gfx, dataCopy );
    cbuf.Bind( gfx );
}
