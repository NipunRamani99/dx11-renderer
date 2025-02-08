#pragma once
#include "IndexedTriangleList.hpp"
#include <DirectXMath.h>
#include <initializer_list>
#include "AABB.hpp"
#include "Vertex.h"
class Cube
{
  public:
    static IndexedTriangleList Make ()
    {
        namespace dx         = DirectX;

        constexpr float side = 1.0f / 2.0f;
        Dvtx::VertexLayout layout;
        layout.Append ( Dvtx::VertexLayout::Position3D );
        Dvtx::VertexBuffer vbuf ( layout );

        vbuf.EmplaceBack ( dx::XMFLOAT3{ -side, -side, -side } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ side, -side, -side } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ -side, side, -side } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ side, side, -side } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ -side, -side, side } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ side, -side, side } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ -side, side, side } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ side, side, side } );

        return { std::move ( vbuf ), { 0, 2, 1, 2, 3, 1, 1, 3, 5, 3, 7, 5, 2, 6, 3, 3, 6, 7,
                                       4, 5, 7, 4, 7, 6, 0, 4, 2, 2, 4, 6, 0, 1, 4, 1, 5, 4 } };
    }

    static IndexedTriangleList MakeWireframe ()
    {
        namespace dx         = DirectX;

        constexpr float side = 1.0f / 2.0f;
        Dvtx::VertexLayout layout;
        layout.Append ( Dvtx::VertexLayout::Position3D );
        Dvtx::VertexBuffer vbuf ( layout );

        vbuf.EmplaceBack ( dx::XMFLOAT3{ -side, -side, -side } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ side, -side, -side } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ -side, side, -side } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ side, side, -side } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ -side, -side, side } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ side, -side, side } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ -side, side, side } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ side, side, side } );

        return { std::move ( vbuf ),
                 {
                     0, 1, 0, 2, 1, 3,       // Bottom front edges
                     4, 5, 4, 6, 5, 7,       // Bottom back edges
                     0, 4, 1, 5,             // Connecting bottom front to back
                     2, 6, 3, 7,             // Top edges
                     2, 3, 6, 7, 0, 2, 1, 3, // Front edges
                     4, 6, 5, 7              // Back edges
                 } };
    }

    static IndexedTriangleList MakeWireframe ( const AABB& aabb )
    {

        namespace dx = DirectX;
        Dvtx::VertexLayout layout;
        layout.Append ( Dvtx::VertexLayout::Position3D );
        Dvtx::VertexBuffer vbuf ( layout );

        vbuf.EmplaceBack ( dx::XMFLOAT3{ aabb.min.x, aabb.min.y, aabb.min.z } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ aabb.min.x, aabb.min.y, aabb.max.z } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ aabb.max.x, aabb.min.y, aabb.max.z } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ aabb.max.x, aabb.min.y, aabb.min.z } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ aabb.min.x, aabb.max.y, aabb.min.z } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ aabb.min.x, aabb.max.y, aabb.max.z } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ aabb.max.x, aabb.max.y, aabb.max.z } );
        vbuf.EmplaceBack ( dx::XMFLOAT3{ aabb.max.x, aabb.max.y, aabb.min.z } );
        std::vector<dx::XMFLOAT3> vertices;
        vertices.resize ( 8 );
        for ( size_t i = 0; i < vbuf.Size (); i++ )
        {
            vertices[i] = vbuf[i].Attr<Dvtx::VertexLayout::Position3D> ();
        }

        return { std::move ( vbuf ),
                 {
                     0, 1, 1, 2, 2, 3, 3, 0, // Bottom Edges
                     4, 5, 5, 6, 6, 7, 7, 4, // Top edges
                     0, 4, 1, 5, 2, 6, 3, 7  // Connecting Bottom and Top Edges
                 } };
    }

    // template<class V>
    // static IndexedTriangleList<V> MakeSkinned()
    //{
    //	namespace dx = DirectX;

    //	constexpr float side = 1.0f / 2.0f;

    //	std::vector<V> vertices(14);

    //	vertices[0].pos = { -side,-side,-side };
    //	vertices[0].tex = { 2.0f / 3.0f,0.0f / 4.0f };
    //	vertices[1].pos = { side,-side,-side };
    //	vertices[1].tex = { 1.0f / 3.0f,0.0f / 4.0f };
    //	vertices[2].pos = { -side,side,-side };
    //	vertices[2].tex = { 2.0f / 3.0f,1.0f / 4.0f };
    //	vertices[3].pos = { side,side,-side };
    //	vertices[3].tex = { 1.0f / 3.0f,1.0f / 4.0f };
    //	vertices[4].pos = { -side,-side,side };
    //	vertices[4].tex = { 2.0f / 3.0f,3.0f / 4.0f };
    //	vertices[5].pos = { side,-side,side };
    //	vertices[5].tex = { 1.0f / 3.0f,3.0f / 4.0f };
    //	vertices[6].pos = { -side,side,side };
    //	vertices[6].tex = { 2.0f / 3.0f,2.0f / 4.0f };
    //	vertices[7].pos = { side,side,side };
    //	vertices[7].tex = { 1.0f / 3.0f,2.0f / 4.0f };
    //	vertices[8].pos = { -side,-side,-side };
    //	vertices[8].tex = { 2.0f / 3.0f,4.0f / 4.0f };
    //	vertices[9].pos = { side,-side,-side };
    //	vertices[9].tex = { 1.0f / 3.0f,4.0f / 4.0f };
    //	vertices[10].pos = { -side,-side,-side };
    //	vertices[10].tex = { 3.0f / 3.0f,1.0f / 4.0f };
    //	vertices[11].pos = { -side,-side,side };
    //	vertices[11].tex = { 3.0f / 3.0f,2.0f / 4.0f };
    //	vertices[12].pos = { side,-side,-side };
    //	vertices[12].tex = { 0.0f / 3.0f,1.0f / 4.0f };
    //	vertices[13].pos = { side,-side,side };
    //	vertices[13].tex = { 0.0f / 3.0f,2.0f / 4.0f };

    //	return{
    //		std::move(vertices),{
    //			0,2,1,   2,3,1,
    //			4,8,5,   5,8,9,
    //			2,6,3,   3,6,7,
    //			4,5,7,   4,7,6,
    //			2,10,11, 2,11,6,
    //			12,3,7,  12,7,13
    //		}
    //	};
    //}
    // template<class V>
    // static IndexedTriangleList<V> MakeIndependent()
    //{
    //	constexpr float side = 1.0f / 2.0f;

    //	std::vector<V> vertices(24);
    //	vertices[0].pos = { -side,-side,-side };// 0 near side
    //	vertices[1].pos = { side,-side,-side };// 1
    //	vertices[2].pos = { -side,side,-side };// 2
    //	vertices[3].pos = { side,side,-side };// 3
    //	vertices[4].pos = { -side,-side,side };// 4 far side
    //	vertices[5].pos = { side,-side,side };// 5
    //	vertices[6].pos = { -side,side,side };// 6
    //	vertices[7].pos = { side,side,side };// 7
    //	vertices[8].pos = { -side,-side,-side };// 8 left side
    //	vertices[9].pos = { -side,side,-side };// 9
    //	vertices[10].pos = { -side,-side,side };// 10
    //	vertices[11].pos = { -side,side,side };// 11
    //	vertices[12].pos = { side,-side,-side };// 12 right side
    //	vertices[13].pos = { side,side,-side };// 13
    //	vertices[14].pos = { side,-side,side };// 14
    //	vertices[15].pos = { side,side,side };// 15
    //	vertices[16].pos = { -side,-side,-side };// 16 bottom side
    //	vertices[17].pos = { side,-side,-side };// 17
    //	vertices[18].pos = { -side,-side,side };// 18
    //	vertices[19].pos = { side,-side,side };// 19
    //	vertices[20].pos = { -side,side,-side };// 20 top side
    //	vertices[21].pos = { side,side,-side };// 21
    //	vertices[22].pos = { -side,side,side };// 22
    //	vertices[23].pos = { side,side,side };// 23

    //	return{
    //		std::move(vertices),{
    //			0,2, 1,    2,3,1,
    //			4,5, 7,    4,7,6,
    //			8,10, 9,  10,11,9,
    //			12,13,15, 12,15,14,
    //			16,17,18, 18,17,19,
    //			20,23,21, 20,22,23
    //		}
    //	};
    //}
};
