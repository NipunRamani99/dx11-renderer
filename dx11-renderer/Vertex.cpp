#include "Vertex.h"
using namespace Dvtx;
VertexBuffer::VertexBuffer( VertexLayout vertexLayout ) : _layout( vertexLayout ) {}
const VertexLayout& VertexBuffer::GetVertexLayout() const
{
    return _layout;
}
size_t VertexBuffer::Size() const
{
    return _data.size() / _layout.Size();
}
const std::uint8_t* VertexBuffer::GetData() const
{
    return _data.data();
}
Vertex VertexBuffer::Back()
{
    assert( _data.size() != 0 );
    return Vertex( _layout, _data.data() + _data.size() - _layout.Size() );
}

Vertex VertexBuffer::Front()
{
    assert( _data.size() != 0 );
    return Vertex( _layout, _data.data() );
}

Vertex VertexBuffer::operator[]( size_t i )
{
    assert( i < Size() );
    return Vertex( _layout, i * _layout.Size() + _data.data() );
}

ConstVertex VertexBuffer::Back() const
{
    assert( _data.size() != 0 );
    return const_cast<VertexBuffer*>( this )->Back();
}

ConstVertex VertexBuffer::Front() const
{
    assert( _data.size() != 0 );
    return const_cast<VertexBuffer*>( this )->Front();
}

ConstVertex VertexBuffer::operator[]( size_t i ) const
{
    assert( _data.size() != 0 );
    return const_cast<VertexBuffer*>( this )->operator[]( i );
}

VertexLayout::Element& VertexLayout::ResolveByIndex( size_t i )
{
    return _elements[i];
}

VertexLayout::Element::size_type VertexLayout::Size() const
{
    return _elements.empty() ? 0 : _elements.back().GetOffsetAfter();
}

const std::vector<VertexLayout::Element>& VertexLayout::getElements() const
{
    return _elements;
}

Dvtx::VertexLayout::Element::Element( ElementType elementType, size_type offset )
    : _elementType( elementType ), _offset( offset )
{
}

VertexLayout::Element::size_type VertexLayout::Element::GetOffsetAfter() const
{
    return _offset + Size();
}

VertexLayout::Element::size_type VertexLayout::Element::GetOffset() const
{
    return _offset;
}

VertexLayout::Element::size_type VertexLayout::Element::Size() const
{
    return sizeOf( _elementType );
}

const VertexLayout::Element::size_type VertexLayout::Element::sizeOf( ElementType type ) const
{
    using namespace DirectX;
    switch( type )
    {
    case Position3D:
    case Float3Color:
    case Normal:
    case Tangent:
    case BiTangent:
        return sizeof( XMFLOAT3 );
    case Position2D:
    case Texture2D:
        return sizeof( XMFLOAT2 );
    case Float4Color:
        return sizeof( XMFLOAT4 );
    case BGRAColor:
        return sizeof( unsigned int );
        break;
    }
    assert( "Incorrect Element Type" && false );
    return 0;
}

const VertexLayout::ElementType VertexLayout::Element::GetType() const
{
    return _elementType;
}
