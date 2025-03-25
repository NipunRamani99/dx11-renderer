#pragma once
#include "ConditionalNoexcept.h"
#include <cassert>
#include <DirectXMath.h>
#include <optional>
#include <vector>
#include <string>
#include <memory>

// master list of leaf types that generates enum elements and various switches etc.
#define LEAF_ELEMENT_TYPES                                                                                             \
    X( Float )                                                                                                         \
    X( Float2 )                                                                                                        \
    X( Float3 )                                                                                                        \
    X( Float4 )                                                                                                        \
    X( Matrix )                                                                                                        \
    X( Bool )
namespace Dcb
{
namespace dx = DirectX;

enum Type
{
#define X( el ) el,
    LEAF_ELEMENT_TYPES
#undef X
        Struct,
    Array,
    Empty
};
// static map of attributes of each leaf type
template <Type type> struct Map
{
    static constexpr bool valid = false;
};
template <> struct Map<Float>
{
    using SysType                     = float;             // type used in the CPU side
    static constexpr size_t hlslSize  = sizeof( SysType ); // size of type on GPU side
    static constexpr const char* code = "F1";              // code used for generating signature of layout
    static constexpr bool valid       = true;              // metaprogramming flag to check validity of Map <param>
};
template <> struct Map<Float2>
{
    using SysType                     = dx::XMFLOAT2;
    static constexpr size_t hlslSize  = sizeof( SysType );
    static constexpr const char* code = "F2";
    static constexpr bool valid       = true;
};
template <> struct Map<Float3>
{
    using SysType                     = dx::XMFLOAT3;
    static constexpr size_t hlslSize  = sizeof( SysType );
    static constexpr const char* code = "F3";
    static constexpr bool valid       = true;
};
template <> struct Map<Float4>
{
    using SysType                     = dx::XMFLOAT4;
    static constexpr size_t hlslSize  = sizeof( SysType );
    static constexpr const char* code = "F4";
    static constexpr bool valid       = true;
};
template <> struct Map<Matrix>
{
    using SysType                     = dx::XMFLOAT4X4;
    static constexpr size_t hlslSize  = sizeof( SysType );
    static constexpr const char* code = "M4";
    static constexpr bool valid       = true;
};
template <> struct Map<Bool>
{
    using SysType                     = bool;
    static constexpr size_t hlslSize  = 4u;
    static constexpr const char* code = "BL";
    static constexpr bool valid       = true;
};

template <typename T> struct ReverseMap
{
    static const constexpr bool valid = false;
};
#define X( el )                                                                                                        \
    template <> struct ReverseMap<Map<el>::SysType>                                                                    \
    {                                                                                                                  \
        static const constexpr bool valid = true;                                                                      \
        static const constexpr Type type  = el;                                                                        \
    };
LEAF_ELEMENT_TYPES
#undef X

class LayoutElement
{
  private:
    // This serves as the polymorphic base class for agregate data types, Struct and Array, to hold extra information.
    class ExtraDataBase
    {
      public:
        virtual ~ExtraDataBase() = default;
    };

    friend class ExtraData;

  private:
    std::optional<size_t> _offset = 0;
    Type _type                    = Empty;
    std::unique_ptr<ExtraDataBase> _pExtraData;

  public:
    LayoutElement() noexcept = default;
    LayoutElement( Type type );

    size_t GetOffsetBegin() const noxnd;
    size_t GetOffsetEnd() const noxnd;
    size_t GetSizeInBytes() const noxnd;

    LayoutElement& T() const noxnd;

    LayoutElement& Add( Type type, std::string key ) noxnd;
    LayoutElement& Set( Type type, size_t size ) noxnd;

    // Check if element is "real"
    bool Exists() const noexcept;

    template <Type type> LayoutElement& Add( const std::string& key ) noxnd
    {
        Add( type, key );
        return *this;
    }

    template <Type type> LayoutElement& Set( const size_t size ) noxnd
    {
        Set( type, size );
        return *this;
    }

    static size_t AdvanceBoundary( size_t offset ) noexcept;
    static size_t AdvanceIfCrossesBoundary( size_t offset, size_t size ) noexcept;
    static bool CrossesBoundary( size_t offset, size_t size ) noexcept;
    static bool ValidateStringName( const std::string& key ) noexcept;
    size_t CalculateOffset( size_t offset, size_t i ) const noexcept;

    // returns singleton instance of empty layout element
    static LayoutElement& GetEmptyElement() noexcept
    {
        static LayoutElement empty{};
        return empty;
    }

    size_t Finalize( const size_t offset ) noxnd;
    size_t FinalizeForStruct( const size_t offset ) noxnd;
    size_t FinalizeForArray( const size_t offset ) noxnd;

    LayoutElement& operator[]( const std::string& key ) noxnd;
    const LayoutElement& operator[]( const std::string& key ) const noxnd;
    std::pair<size_t, LayoutElement*> operator[]( const size_t i ) noxnd;
    std::pair<size_t, LayoutElement*> operator[]( const size_t i ) const noxnd;

    std::string GetSignature() const noxnd;
    std::string GetSignatureForStruct() const noxnd;
    std::string GetSignatureForArray() const noxnd;

    		// returns offset of leaf types for read/write purposes w/ typecheck in Debug
		template<typename T>
		size_t Resolve() const noxnd
		{
			switch( _type )
			{
			#define X(el) case el: assert(typeid(Map<el>::SysType) == typeid(T)); return *_offset;
			LEAF_ELEMENT_TYPES
			#undef X
			default:
				assert( "Tried to resolve non-leaf element" && false );
				return 0u;
			}
		}
};

class Layout
{
    friend class LayoutCodex;
    friend class Buffer;

  protected:
    std::shared_ptr<LayoutElement> _pRoot;

  protected:
    Layout( std::shared_ptr<LayoutElement> pRoot ) noexcept;

  public:
    size_t GetSizeInBytes() const noexcept;
    const std::string GetSignature() const noexcept;
};

class RawLayout : public Layout
{
    friend class LayoutCodex;

  public:
    RawLayout() noexcept;

    // key into root struct
    LayoutElement& operator[]( const std::string& key ) noxnd;

    // add an element to the root struct
    template <Type type> LayoutElement& Add( const std::string& key ) noxnd
    {
        return _pRoot->Add<type>( key );
    }

    const std::string GetSignature()
    {
        return _pRoot->GetSignature();
    }

  private:
    // reset this object with an empty struct at its root
    void ClearRoot() noexcept;

    // finalize the layout and return and then relinquish (by yielding the root layout element)
    std::shared_ptr<LayoutElement> DeliverRoot() noexcept;
};
class CookedLayout : public Layout
{
    friend class LayoutCodex;
    friend class Buffer;

  public:
    // key into the root Struct (const to disable mutation of the layout)
    const LayoutElement& operator[]( const std::string& key ) const noxnd;
    // get a share on layout tree root
    std::shared_ptr<LayoutElement> ShareRoot() const noexcept;

  private:
    // this ctor is used by LayoutCodex to returen codex layouts
    CookedLayout( std::shared_ptr<LayoutElement> pRoot ) noexcept;

    // use to pilfer the layout tree
    std::shared_ptr<LayoutElement> RelinquishRoot() noexcept;
};

class ConstElementRef
{
    friend class Buffer;
    friend class ElementRef;

  public:
    class Ptr
    {
        friend ConstElementRef;

      public:
        template <typename T> operator const T*() const noxnd
        {
            static_assert( ReverseMap<std::remove_const_t<T>>::valid,
                           "Unsupported SysType used in pointer conversion" );
            return &static_cast<const T&>( *ref );
        }

      private:
        Ptr( const ConstElementRef* ref ) noexcept;
        const ConstElementRef* ref;
    };

  private:
    size_t offset;
    const LayoutElement* pLayout;
    const char* pBytes;
    ConstElementRef( const LayoutElement* pLayout, const char* pBytes, size_t offset ) noexcept;

  public:
    bool Exists() const noexcept;

    ConstElementRef operator[]( const std::string& key ) const noxnd;
    ConstElementRef operator[]( size_t index ) const noxnd;

    Ptr operator&() const noxnd;

    template <typename T> operator const T&() const noxnd
    {
        static_assert( ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in conversion" );
        return *reinterpret_cast<const T*>( pBytes + offset + pLayout->Resolve<T>() );
    }
};

class ElementRef
{
    friend class Buffer;

  private:
    const size_t _offset                 = 0;
    const LayoutElement* _pLayoutElement = nullptr; // Do not index into _pLayoutElement without dereferencing it.
    char* _buffer                        = nullptr;

    // refs should only be constructable by other refs or by the buffer
    ElementRef( size_t offset, char* buffer, const LayoutElement* pLayoutElement );

  public:
    class Ptr
    {
        friend class ElementRef;

      private:
        Ptr( ElementRef* ref ) noexcept;
        ElementRef* ref;

      public:
        // conversion to read/write pointer  to supported SysType
        template <typename T> operator T*() const noxnd
        {
            static_assert( ReverseMap<std::remove_const_t<T>>::valid,
                           "Unsupported SysType used in pointer conversion" );
            return &static_cast<T&>( *ref );
        }
    };

  public:
    operator ConstElementRef() const noexcept;
    bool Exists() const noexcept;

    ElementRef operator[](const std::string& key ) const;

    ElementRef operator[](const char* key ) const;
      

    ElementRef operator[]( size_t i ) const ;

    // optionally set value if not an empty Ref
    template <typename S> bool SetIfExists( const S& val ) noxnd
    {
        if( Exists() )
        {
            *this = val;
            return true;
        }
        return false;
    }
    Ptr operator&() const noxnd;
    // conversion for reading/writing as a supported SysType
    template <typename T> operator T&() const noxnd
    {
        static_assert( ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in conversion" );
        return *reinterpret_cast<T*>( _buffer + _offset + _pLayoutElement->Resolve<T>() );
    }
    // assignment for writing to as a supported SysType
    template <typename T> T& operator=( const T& rhs ) const noxnd
    {
        static_assert( ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in assignment" );
        return static_cast<T&>( *this ) = rhs;
    }
};

class Buffer
{
  private:
    std::shared_ptr<LayoutElement> _root;
    std::vector<char> _buffer;

  public:
    Buffer( const CookedLayout& layout ) noxnd;
    Buffer( CookedLayout&& layout ) noxnd;
    Buffer( RawLayout&& layout ) noxnd;
    Buffer( const Buffer& ) noexcept;
    Buffer( Buffer&& ) noexcept;

    ElementRef operator[]( std::string key );
    ConstElementRef operator[]( const std::string& key ) const noxnd;

    const std::uint8_t* GetData() const;
    const size_t GetSizeInBytes() const;
    void CopyFrom( const Buffer& ) noxnd;
    const LayoutElement& GetRootLayoutElement() const noexcept;
    std::shared_ptr<LayoutElement> ShareLayoutRoot() const noexcept;
};
} // namespace Dcb

#ifndef DCB_IMPL_SOURCE
#undef LEAF_ELEMENT_TYPES
#endif
