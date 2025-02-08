#pragma once
#include "AABB.hpp"
#include "IndexBuffer.hpp"
#include "VertexShader.hpp"
#include "PixelShader.hpp"
#include "VertexBuffer.hpp"
#include "Vertex.h"
#include "Cube.hpp"
class AABBVisualisation : public Drawable
{
  private:
    AABB _aabb;
    DirectX::XMMATRIX _transform;
    float _scale_x = 0.0f;
    float _scale_y = 0.0f;
    float _scale_z = 0.0f;

  public:
    AABBVisualisation( Graphics& gfx, const AABB& aabb, const std::string& name )
        : _aabb( aabb ), _transform( DirectX::XMMatrixIdentity() ), _scale_x( aabb.max.x - aabb.min.x ),
          _scale_y( aabb.max.y - aabb.min.y ), _scale_z( aabb.max.z - aabb.min.z )
    {
        struct Vertex
        {
            DirectX::XMFLOAT3 pos;
        };
        IndexedTriangleList cube = Cube::MakeWireframe( aabb );
        auto buf                 = std::move( cube.vertices );
        std::vector<DirectX::XMFLOAT3> vertices;
        vertices.resize( 8 );
        for ( size_t i = 0; i < buf.Size(); i++ )
        {
            vertices[i] = buf[i].Attr<Dvtx::VertexLayout::Position3D>();
        }

        using namespace Bind;
        auto bindable = VertexBuffer::Resolve( gfx, name + "AABB", buf );
        AddBind( bindable );

        AddBind( IndexBuffer::Resolve( gfx, name + "AABB", cube.indices ) );

        auto vs   = VertexShader::Resolve( gfx, "SolidVS.cso" );

        auto vsbc = vs->GetBytecode();
        AddBind( vs );
        auto layout = buf.GetVertexLayout();
        AddBind( InputLayout::Resolve( gfx, layout, vsbc ) );

        AddBind( PixelShader::Resolve( gfx, "SolidPS.cso" ) );

        AddBind( std::make_shared<TransformCbuf>( gfx, *this ) );

        AddBind( Topology::Resolve( gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST ) );
    }

    void SetTransform( const DirectX::XMFLOAT4X4& transform )
    {
        _transform = DirectX::XMLoadFloat4x4( &transform );
    }

    void SetTransform( const DirectX::XMMATRIX& transform )
    {
        _transform = transform;
    }

    void SetAABB( const AABB aabb )
    {
        _aabb    = aabb;
        _scale_x = aabb.max.x - aabb.min.x;
        _scale_y = aabb.max.y - aabb.min.y;
        _scale_z = aabb.max.z - aabb.min.z;
    }

    DirectX::XMMATRIX GetTransformXM() const noexcept
    {
        return _transform;
    }

    void Update( float ) noexcept {}
};
