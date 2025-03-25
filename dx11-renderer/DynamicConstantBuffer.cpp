#define DCB_IMPL_SOURCE

#include "DynamicConstantBuffer.hpp"
#include <string>
#include <algorithm>
#include <cctype>
#include "LayoutCodex.hpp"
namespace Dcb
{
#define RESOLVE_STRUCT( pExtraData ) static_cast<ExtraData::Struct&>( *pExtraData )
#define RESOLVE_ARRAY( pExtraData ) static_cast<ExtraData::Array&>( *pExtraData )

struct ExtraData
{
    struct Struct : public LayoutElement::ExtraDataBase
    {
        std::vector<std::pair<std::string, LayoutElement>> _layoutElements;
    };
    struct Array : public LayoutElement::ExtraDataBase
    {
        std::optional<LayoutElement> _layoutElement;
        size_t size = 0;
    };
};

LayoutElement::LayoutElement( Type type ) : _type( type )
{
    assert( _type != Empty );
    switch( _type )
    {
    case Struct:
        _pExtraData = std::make_unique<ExtraData::Struct>();
        break;
    case Array:
        _pExtraData = std::make_unique<ExtraData::Array>();
        break;
    }
}

size_t LayoutElement::GetOffsetBegin() const noxnd
{
    return *_offset;
}

size_t LayoutElement::GetOffsetEnd() const noxnd
{
    switch( _type )
    {
#define X( el )                                                                                                        \
    case el:                                                                                                           \
        return *_offset + Map<el>::hlslSize;
        LEAF_ELEMENT_TYPES
#undef X
    case Struct: {
        ExtraData::Struct& s       = static_cast<ExtraData::Struct&>( *_pExtraData );
        LayoutElement& lastElement = s._layoutElements.back().second;
        return AdvanceBoundary( lastElement.GetOffsetEnd() );
    }
    case Array: {
        ExtraData::Array& a = static_cast<ExtraData::Array&>( *_pExtraData );
        return *_offset + a.size * AdvanceBoundary( ( *a._layoutElement ).GetSizeInBytes() );
    }
    default:
        assert( false && "GetOffsetEnd called on invalid LayoutElement" );
    }
    return 0;
}

size_t LayoutElement::GetSizeInBytes() const noxnd
{
    return GetOffsetEnd() - GetOffsetBegin();
}

LayoutElement& LayoutElement::T() const noxnd
{
    assert( _type == Array && "Cannot call T() on a non-array LayoutElement" );
    ExtraData::Array& a = static_cast<ExtraData::Array&>( *_pExtraData );
    return *a._layoutElement;
}

bool LayoutElement::Exists() const noexcept
{
    return _type != Empty;
}

LayoutElement& LayoutElement::Add( Type type, std::string key ) noxnd
{
    assert( _type == Struct && "Cannot call Add() on a non-struct LayoutElement" );
    ExtraData::Struct& s = static_cast<ExtraData::Struct&>( *_pExtraData );
    assert( ValidateStringName( key ) && "Provided key is not valid" );
    auto& layoutElements = s._layoutElements;
    for( const auto& el : layoutElements )
    {
        assert( el.first != key && "Unique key constraint violated" );
    }
    layoutElements.emplace_back( key, LayoutElement( type ) );
    return *this;
}
LayoutElement& LayoutElement::Set( Type type, size_t size ) noxnd
{
    assert( _type == Array && "Cannot call Set() on a non-struct LayoutElement" );
    ExtraData::Array& a = static_cast<ExtraData::Array&>( *_pExtraData );
    a.size              = size;
    a._layoutElement    = LayoutElement( type );
    return *this;
}

size_t LayoutElement::AdvanceBoundary( size_t offset ) noexcept
{
    return offset + ( 16 - offset % 16 ) % 16;
}

size_t LayoutElement::AdvanceIfCrossesBoundary( size_t offset, size_t size ) noexcept
{
    return CrossesBoundary( offset, size ) ? AdvanceBoundary( offset ) : offset;
}

bool LayoutElement::CrossesBoundary( size_t offset, size_t size ) noexcept
{
    const auto end       = offset + size;
    const auto pageStart = offset / 16u;
    const auto pageEnd   = end / 16u;
    return ( pageStart != pageEnd && pageEnd % 16u != 0 ) || ( size > 16u );
}

bool LayoutElement::ValidateStringName( const std::string& key ) noexcept
{
    return !key.empty() && !std::isdigit( key.front() ) &&
           std::all_of( key.begin(), key.end(), []( char c ) { return std::isalnum( c ) || c == '_'; } );
}

size_t LayoutElement::Finalize( const size_t offset ) noxnd
{
    switch( _type )
    {
#define X( el )                                                                                                        \
    case el:                                                                                                           \
        _offset = AdvanceIfCrossesBoundary( offset, Map<el>::hlslSize );                                               \
        return *_offset + Map<el>::hlslSize;
        LEAF_ELEMENT_TYPES
#undef X
    case Struct:
        return FinalizeForStruct( offset );
    case Array:
        return FinalizeForArray( offset );
    default:
        assert( false && "Finalize called on an invalid LayoutElement type." );
    }
    return 0;
}

size_t LayoutElement::FinalizeForStruct( size_t offset ) noxnd
{
    assert( _type == Struct && "Cannot finalize a non-struct LayoutElement" );
    _offset              = AdvanceBoundary( offset );
    ExtraData::Struct& s = static_cast<ExtraData::Struct&>( *_pExtraData );
    size_t nextOffset    = *_offset;
    for( auto& elem : s._layoutElements )
    {
        nextOffset = elem.second.Finalize( nextOffset );
    }
    return nextOffset;
}

size_t LayoutElement::FinalizeForArray( size_t offset ) noxnd
{
    assert( _type == Array && "Cannot finalize a non-array LayoutElement" );
    ExtraData::Array& a = static_cast<ExtraData::Array&>( *_pExtraData );
    _offset             = AdvanceBoundary( offset );
    a._layoutElement->Finalize( *_offset );
    return GetOffsetEnd();
}

size_t LayoutElement::CalculateOffset( size_t offset, size_t i ) const noexcept
{
    return offset + AdvanceBoundary( GetSizeInBytes() ) * i;
}

LayoutElement& LayoutElement::operator[]( const std::string& key ) noxnd
{
    assert( _type == Struct && "Cannot key into a non-struct LayoutElement" );
    ExtraData::Struct& s = RESOLVE_STRUCT( _pExtraData );
    auto& layoutElements = s._layoutElements;
    for( auto& elem : layoutElements )
    {
        if( key == elem.first )
            return elem.second;
    }
    assert( "Could not find element for the given key" );
    return GetEmptyElement();
}

const LayoutElement& LayoutElement::operator[]( const std::string& key ) const noxnd
{
    assert( _type == Struct && "Cannot key into a non-struct LayoutElement" );
    ExtraData::Struct& s = RESOLVE_STRUCT( _pExtraData );
    auto& layoutElements = s._layoutElements;
    for( auto& elem : layoutElements )
    {
        if( key == elem.first )
            return elem.second;
    }
    assert( "Could not find element for the given key" );
    return GetEmptyElement();
}

std::pair<size_t, LayoutElement*> LayoutElement::operator[]( const size_t i ) noxnd
{
    assert( _type == Array && "Cannot key into a non-array LayoutElement" );
    ExtraData::Array& a = RESOLVE_ARRAY( _pExtraData );
    assert( i < a.size );
    LayoutElement& elem = *a._layoutElement;
    return { elem.CalculateOffset( *_offset, i ), &elem };
}

std::pair<size_t, LayoutElement*> LayoutElement::operator[]( const size_t i ) const noxnd
{
    assert( _type == Array && "Cannot key into a non-array LayoutElement" );
    ExtraData::Array& a = RESOLVE_ARRAY( _pExtraData );
    assert( i < a.size );
    LayoutElement& elem = *a._layoutElement;
    return { elem.CalculateOffset( *_offset, i ), &elem };
}

std::string LayoutElement::GetSignature() const noxnd
{
    switch( _type )
    {
#define X( el )                                                                                                        \
    case el:                                                                                                           \
        return Map<el>::code;
        LEAF_ELEMENT_TYPES
#undef X
    case Struct:
        return GetSignatureForStruct();
    case Array:
        return GetSignatureForArray();
    default:
        assert( false && "Calling GetSignature on an invalid LayoutElement" );
        break;
    }
    return "";
}

std::string LayoutElement::GetSignatureForStruct() const noxnd
{
    using namespace std::string_literals;
    assert( _type == Struct && "Cannot call this method on a non-struct LayoutElement type" );
    ExtraData::Struct& data = RESOLVE_STRUCT( _pExtraData );
    std::string signature   = "St{"s;
    for( auto& element : data._layoutElements )
    {
        signature += element.second.GetSignature();
    }
    signature += "}"s;
    return signature;
}

std::string LayoutElement::GetSignatureForArray() const noxnd
{
    using namespace std::string_literals;
    assert( _type == Array && "Cannot call this method on a non-array LayoutElement type" );
    ExtraData::Array& data = RESOLVE_ARRAY( _pExtraData );
    std::string signature =
        "Arr"s + "#"s + std::to_string( data.size ) + "{"s + data._layoutElement->GetSignature() + "}"s;
    return signature;
}

Layout::Layout( std::shared_ptr<LayoutElement> pRoot ) noexcept : _pRoot( std::move( pRoot ) ) {}

size_t Layout::GetSizeInBytes() const noexcept
{
    return _pRoot->GetSizeInBytes();
}

const std::string Layout::GetSignature() const noexcept
{
    return _pRoot->GetSignature();
}

RawLayout::RawLayout() noexcept : Layout( std::shared_ptr<LayoutElement>{ new LayoutElement( Struct ) } ) {}

LayoutElement& RawLayout::operator[]( const std::string& key ) noxnd
{
    return ( *_pRoot )[key];
}

std::shared_ptr<LayoutElement> RawLayout::DeliverRoot() noexcept
{
    auto temp = std::move( _pRoot );
    temp->Finalize( 0 );
    *this = RawLayout();
    return std::move( temp );
}

void RawLayout::ClearRoot() noexcept
{
    *this = RawLayout();
}

CookedLayout::CookedLayout( std::shared_ptr<LayoutElement> layoutElement ) noexcept
    : Layout( std::move( layoutElement ) )
{
}

std::shared_ptr<LayoutElement> CookedLayout::ShareRoot() const noexcept
{
    return _pRoot;
}

std::shared_ptr<LayoutElement> CookedLayout::RelinquishRoot() noexcept
{
    return std::move( _pRoot );
}

const LayoutElement& CookedLayout::operator[]( const std::string& key ) const noxnd
{
    return ( *_pRoot )[key];
}

bool ConstElementRef::Exists() const noexcept
{
    return pLayout->Exists();
}
ConstElementRef ConstElementRef::operator[]( const std::string& key ) const noxnd
{
    return { &( *pLayout )[key], pBytes, offset };
}
ConstElementRef ConstElementRef::operator[]( size_t index ) const noxnd
{
    const auto arrayElement = ( *pLayout )[index];
    return { arrayElement.second, pBytes, arrayElement.first };
}
ConstElementRef::Ptr ConstElementRef::operator&() const noxnd
{
    return Ptr{ this };
}
ConstElementRef::ConstElementRef( const LayoutElement* pLayout, const char* pBytes, size_t offset ) noexcept
    : offset( offset ), pLayout( pLayout ), pBytes( pBytes )
{
}
ConstElementRef::Ptr::Ptr( const ConstElementRef* ref ) noexcept : ref( ref ) {}

ElementRef::ElementRef( size_t offset, char* buffer, const LayoutElement* pLayoutElement )
    : _offset( offset ), _buffer( buffer ), _pLayoutElement( pLayoutElement )
{
}
ElementRef::Ptr ElementRef::operator&() const noxnd
{
    return Ptr{ const_cast<ElementRef*>( this ) };
}
ElementRef ElementRef::operator[]( const std::string& key ) const
{
    return { _offset, _buffer, &( ( *_pLayoutElement )[key] ) };
}

ElementRef ElementRef::operator[]( const char* key ) const
{
    return { _offset, _buffer, &( ( *_pLayoutElement )[key] ) };
}

ElementRef ElementRef::operator[]( size_t i ) const
{
    auto arrayElement = ( *_pLayoutElement )[i];
    return { arrayElement.first, _buffer, arrayElement.second };
}

bool ElementRef::Exists() const noexcept
{
    return _pLayoutElement->Exists();
}

ElementRef Buffer::operator[]( std::string key )
{
    return { 0, _buffer.data(), &( *_root )[key] };
}
ElementRef::Ptr::Ptr( ElementRef* ref ) noexcept : ref( ref ) {}

Buffer::Buffer( CookedLayout&& lay ) noxnd : _root( lay.RelinquishRoot() ), _buffer( _root->GetOffsetEnd() ) {}

Buffer::Buffer( RawLayout&& lay ) noxnd : Buffer( LayoutCodex::Resolve( std::move( lay ) ) ) {}

Buffer::Buffer( const CookedLayout& lay ) noxnd : _root( lay.ShareRoot() ), _buffer( _root->GetOffsetEnd() ) {}

Buffer::Buffer( const Buffer& buf ) noexcept : _root( buf._root ), _buffer( _root->GetOffsetEnd() ) {}

Buffer::Buffer( Buffer&& buf ) noexcept : _root( std::move( buf._root ) ), _buffer( std::move( buf._buffer ) ) {}

const std::uint8_t* Buffer::GetData() const
{
    return reinterpret_cast<const std::uint8_t*>( _buffer.data() );
}

const size_t Buffer::GetSizeInBytes() const
{
    return _root->GetSizeInBytes();
}

const LayoutElement& Buffer::GetRootLayoutElement() const noexcept
{
    return *_root;
}

std::shared_ptr<LayoutElement> Buffer::ShareLayoutRoot() const noexcept
{
    return _root;
}

void Buffer::CopyFrom( const Buffer& other ) noxnd
{
    assert( &GetRootLayoutElement() == &other.GetRootLayoutElement() );
    std::copy( other._buffer.begin(), other._buffer.end(), _buffer.begin() );
}

} // namespace Dcb
