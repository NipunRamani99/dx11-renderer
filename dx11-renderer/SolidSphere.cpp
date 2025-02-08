#include "SolidSphere.hpp"
#include "BindableBase.hpp"
#include "GraphicsThrowMacros.h"
#include "Sphere.hpp"

SolidSphere::SolidSphere ( Graphics& gfx, float radius )
{
    namespace dx = DirectX;
    using namespace Bind;

    Dvtx::VertexLayout layout;
    Dvtx::VertexBuffer buf ( layout.Append<Dvtx::VertexLayout::Position3D> () );
    struct Vertex
    {
        dx::XMFLOAT3 pos;
    };
    auto model = Sphere::MakeSolid ();
    model.Transform ( dx::XMMatrixScaling ( radius, radius, radius ) );

    AddBind ( VertexBuffer::Resolve ( gfx, "SolidSphere", model.vertices ) );
    AddBind ( IndexBuffer::Resolve ( gfx, "SolidSphere", model.indices ) );

    auto pvs   = VertexShader::Resolve ( gfx, "SolidVS.cso" );
    auto pvsbc = pvs->GetBytecode ();
    AddBind ( std::move ( pvs ) );

    AddBind ( PixelShader::Resolve ( gfx, "SolidPS.cso" ) );

    struct PSColorConstant
    {
        dx::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        static std::string GetId ()
        {
            return "PSColorConstant";
        }
    } colorConst;
    AddBind ( PixelConstantBuffer<PSColorConstant>::Resolve ( gfx, colorConst ) );

    AddBind ( InputLayout::Resolve ( gfx, layout, pvsbc ) );
    AddBind ( Bind::Blender::Resolve ( gfx, false ) );
    AddBind ( Topology::Resolve ( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );
    AddBind ( std::make_shared<TransformCbuf> ( gfx, *this ) );
}

void SolidSphere::SetPos ( DirectX::XMFLOAT3 pos ) noexcept
{
    this->_pos = pos;
}

DirectX::XMMATRIX SolidSphere::GetTransformXM () const noexcept
{
    return DirectX::XMMatrixTranslation ( _pos.x, _pos.y, _pos.z );
}
