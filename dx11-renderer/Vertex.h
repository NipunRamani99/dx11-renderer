#pragma once
#include <vector>
#include <DirectXMath.h>
#include <type_traits>
#include "Graphics.hpp"
#include "Color.hpp"
namespace Dvtx
{
struct BGRAColor
{
    unsigned char a;
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

class VertexLayout
{

  public:
    enum ElementType
    {
        Position2D,
        Position3D,
        Texture2D,
        Normal,
        Tangent,
        BiTangent,
        Float3Color,
        Float4Color,
        BGRAColor
    };
    template <ElementType> struct Map;
    template <> struct Map<Position2D>
    {
        using SysType                                 = DirectX::XMFLOAT2;
        static constexpr const DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
        static constexpr const char* semantic         = "Position";
        static constexpr const char* code             = "P2";
    };
    template <> struct Map<Position3D>
    {
        using SysType                                 = DirectX::XMFLOAT3;
        static constexpr const DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        static constexpr const char* semantic         = "Position";
        static constexpr const char* code             = "P3";
    };
    template <> struct Map<Texture2D>
    {
        using SysType                                 = DirectX::XMFLOAT2;
        static constexpr const DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
        static constexpr const char* semantic         = "TexCoord";
        static constexpr const char* code             = "T2";
    };
    template <> struct Map<Normal>
    {
        using SysType                                 = DirectX::XMFLOAT3;
        static constexpr const DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        static constexpr const char* semantic         = "Normal";
        static constexpr const char* code             = "N3";
    };
    template <> struct Map<Tangent>
    {
        using SysType                                 = DirectX::XMFLOAT3;
        static constexpr const DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        static constexpr const char* semantic         = "Tangent";
        static constexpr const char* code             = "TAN3";
    };
    template <> struct Map<BiTangent>
    {
        using SysType                                 = DirectX::XMFLOAT3;
        static constexpr const DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        static constexpr const char* semantic         = "BiTangent";
        static constexpr const char* code             = "BITAN3";
    };
    template <> struct Map<Float3Color>
    {
        using SysType                                 = DirectX::XMFLOAT3;
        static constexpr const DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        static constexpr const char* semantic         = "Color";
        static constexpr const char* code             = "C3";
    };
    template <> struct Map<Float4Color>
    {
        using SysType                                 = DirectX::XMFLOAT4;
        static constexpr const DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
        static constexpr const char* semantic         = "Color";
        static constexpr const char* code             = "C4";
    };
    template <> struct Map<BGRAColor>
    {
        using SysType                                 = DirectX::XMFLOAT4;
        static constexpr const DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        static constexpr const char* semantic         = "Color";
        static constexpr const char* code             = "C8";
    };

    class Element
    {

      public:
        using size_type = std::size_t;

      private:
        size_type _offset = 0;
        ElementType _elementType;

      public:
        Element( ElementType elementType, size_type offset );

        size_type GetOffsetAfter() const;

        size_type GetOffset() const;

        size_type Size() const;

        const size_type sizeOf( ElementType type ) const;

        const ElementType GetType() const;

        template <ElementType type> const D3D11_INPUT_ELEMENT_DESC GenerateDesc( size_t offset ) const
        {
            return { Map<type>::semantic, 0, Map<type>::dxgiFormat, 0, (UINT)offset, D3D11_INPUT_PER_VERTEX_DATA, 0 };
        }

        const D3D11_INPUT_ELEMENT_DESC GetDesc() const
        {
            switch( _elementType )
            {
            case Position2D:
                return GenerateDesc<Position2D>( _offset );
            case Position3D:
                return GenerateDesc<Position3D>( _offset );
            case Texture2D:
                return GenerateDesc<Texture2D>( _offset );
            case Normal:
                return GenerateDesc<Normal>( _offset );
            case Tangent:
                return GenerateDesc<Tangent>( _offset );
            case BiTangent:
                return GenerateDesc<BiTangent>( _offset );
            case Float3Color:
                return GenerateDesc<Float3Color>( _offset );
            case Float4Color:
                return GenerateDesc<Float4Color>( _offset );
            case BGRAColor:
                return GenerateDesc<BGRAColor>( _offset );
            }
            assert( "Invalid Element Type" );
            return { "INVALID", 0, DXGI_FORMAT_UNKNOWN, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
        }

        const char* GetCode() const noexcept
        {
            switch( _elementType )
            {
            case Position2D:
                return Map<Position2D>::code;
            case Position3D:
                return Map<Position3D>::code;
            case Texture2D:
                return Map<Texture2D>::code;
            case Normal:
                return Map<Normal>::code;
            case Tangent:
                return Map<Tangent>::code;
            case BiTangent:
                return Map<BiTangent>::code;
            case Float3Color:
                return Map<Float3Color>::code;
            case Float4Color:
                return Map<Float4Color>::code;
            case BGRAColor:
                return Map<BGRAColor>::code;
            }
            assert( "Invalid element type" && false );
            return "Invalid";
        }
    };

  private:
    std::vector<Element> _elements;

  public:
    template <ElementType elementType> VertexLayout& Append()
    {
        Element::size_type nextOffset = _elements.size() == 0 ? 0 : _elements.back().GetOffsetAfter();
        _elements.emplace_back( elementType, nextOffset );
        return *this;
    }
    VertexLayout& Append( ElementType elementType )
    {
        Element::size_type nextOffset = _elements.size() == 0 ? 0 : _elements.back().GetOffsetAfter();
        _elements.emplace_back( elementType, nextOffset );
        return *this;
    }

    template <ElementType elementType> Element& Resolve()
    {
        for( auto& e : _elements )
        {
            if( elementType == e.GetType() )
            {
                return e;
            }
        }
        assert( "Could not resolve element type" && false );
        return _elements.front();
    }

    Element& ResolveByIndex( size_t i );

    Element::size_type Size() const;

    const std::vector<Element>& getElements() const;

    const size_t GetElementsCount() const
    {
        return _elements.size();
    }

    const std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayout() const
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
        layout.reserve( _elements.size() );
        for( auto e : _elements )
        {
            layout.push_back( e.GetDesc() );
        }
        return layout;
    }

    const std::string GetCode() const
    {
        std::string code = "";
        for( auto e : _elements )
        {
            code += e.GetCode();
        }
        return code;
    }
};

class Vertex
{
  private:
    std::uint8_t* _pdata = nullptr;
    VertexLayout& _layout;
    friend class VertexBuffer;

  public:
    Vertex( VertexLayout& layout, std::uint8_t* data ) : _pdata( data ), _layout( layout )
    {
        assert( data != nullptr );
    }

    template <VertexLayout::ElementType elementType> auto& Attr()
    {
        using namespace DirectX;
        VertexLayout::Element& elem = _layout.Resolve<elementType>();
        std::uint8_t* pattr         = elem.GetOffset() + _pdata;
        return *reinterpret_cast<VertexLayout::Map<elementType>::SysType*>( pattr );
    }

    template <typename T> void SetAttributeByIndex( size_t i, T&& val )
    {
        using namespace DirectX;
        const auto& element = _layout.ResolveByIndex( i );
        auto pattr          = _pdata + element.GetOffset();
        auto type           = element.GetType();
        switch( type )
        {
        case VertexLayout::ElementType::Position3D:
        case VertexLayout::ElementType::Float3Color:
        case VertexLayout::ElementType::Normal:
        case VertexLayout::ElementType::Tangent:
        case VertexLayout::ElementType::BiTangent:
            SetAttribute<XMFLOAT3>( pattr, std::forward<T>( val ) );
            break;
        case VertexLayout::ElementType::Position2D:
        case VertexLayout::ElementType::Texture2D:
            SetAttribute<XMFLOAT2>( pattr, std::forward<T>( val ) );
            break;
        case VertexLayout::ElementType::Float4Color:
            SetAttribute<XMFLOAT4>( pattr, std::forward<T>( val ) );
            break;
        case VertexLayout::ElementType::BGRAColor:
            SetAttribute<unsigned int*>( pattr, std::forward<T>( val ) );
            break;
        default:
            assert( "SetAttributeByIndex failed with incorrect element type" && false );
        }
    }

    template <typename First, typename... Rest> void SetAttributeByIndex( size_t i, First&& first, Rest&&... rest )
    {
        SetAttributeByIndex( i, std::forward<First>( first ) );
        SetAttributeByIndex( i + 1, std::forward<Rest>( rest )... );
    }

    template <typename Dest, typename Source> void SetAttribute( std::uint8_t* pDest, Source&& val )
    {
        if constexpr( std::is_assignable<Source, Dest>::value )
        {
            *reinterpret_cast<Dest*>( pDest ) = val;
        }
        else
        {
            assert( "The parameter type Source is not assignable to type Dest." && false );
        }
    }
};

class ConstVertex
{
  private:
    Vertex _vertex;

  public:
    ConstVertex( const Vertex& vertex ) : _vertex( vertex ) {}

    template <VertexLayout::ElementType elementType> const auto& Attr() const
    {
        return const_cast<Vertex&>( _vertex ).Attr<elementType>();
    }
};

class VertexBuffer
{
  private:
    std::vector<std::uint8_t> _data;
    VertexLayout _layout;

  public:
    VertexBuffer( VertexLayout vertexLayout );

    const VertexLayout& GetVertexLayout() const;

    size_t Size() const;

    const std::uint8_t* GetData() const;

    template <typename... Params> void EmplaceBack( Params&&... params )
    {
        assert( sizeof...( params ) == _layout.GetElementsCount() &&
                "Param count doesn't match number of vertex elements" );
        _data.resize( _data.size() + _layout.Size() );
        Back().SetAttributeByIndex( 0u, std::forward<Params>( params )... );
    }

    Vertex Back();

    Vertex Front();

    Vertex operator[]( size_t i );

    ConstVertex Back() const;

    ConstVertex Front() const;

    ConstVertex operator[]( size_t i ) const;
};
} // namespace Dvtx
