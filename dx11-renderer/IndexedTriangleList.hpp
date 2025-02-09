#pragma once
#include <vector>
#include <DirectXMath.h>
#include "Vertex.h"

class IndexedTriangleList
{
  public:
    Dvtx::VertexBuffer vertices;
    std::vector<unsigned short> indices;

  public:
    IndexedTriangleList() = default;
    IndexedTriangleList( Dvtx::VertexBuffer verts_in, std::vector<unsigned short> indices_in )
        : vertices( std::move( verts_in ) ), indices( std::move( indices_in ) )
    {
        assert( vertices.Size() > 2 );
    }
    void Transform( DirectX::FXMMATRIX matrix )
    {
        using Elements = Dvtx::VertexLayout::ElementType;
        for( size_t i = 0; i < vertices.Size(); i++ )
        {
            auto v                      = vertices[i];
            const DirectX::XMVECTOR pos = DirectX::XMLoadFloat3( &v.Attr<Dvtx::VertexLayout::Position3D>() );
            DirectX::XMStoreFloat3( &v.Attr<Dvtx::VertexLayout::Position3D>(),
                                    DirectX::XMVector3Transform( pos, matrix ) );
        }
    }

    // void SetNormalsIndependentFlat() noexcept(!IS_DEBUG)
    //{
    //	using namespace DirectX;
    //	assert(indices.size() % 3 == 0 && indices.size() > 0);
    //	for (size_t i = 0; i < indices.size(); i += 3) {
    //		auto& v0 = vertices[indices[i]];
    //		auto& v1 = vertices[indices[i+1]];
    //		auto& v2 = vertices[indices[i+2]];
    //		const auto p0 = XMLoadFloat3(&v0.pos);
    //		const auto p1 = XMLoadFloat3(&v1.pos);
    //		const auto p2 = XMLoadFloat3(&v2.pos);
    //		const auto n = XMVector3Normalize(XMVector3Cross((p1 - p0),  (p2 - p0)));
    //		XMStoreFloat3(&v0.n, n);
    //		XMStoreFloat3(&v1.n, n);
    //		XMStoreFloat3(&v2.n, n);
    //	}
    // }
};
