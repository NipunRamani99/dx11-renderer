#pragma once
#include <vector>
#include <DirectXMath.h>
#include <type_traits>
namespace hw3dexp {
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
			Float3Color,
			Float4Color,
			BGRAColor
		};

		class Element
		{
		public:
			using size_type = std::size_t;

		private:
			size_type _offset = 0;
			ElementType _elementType;

		public:
			Element(ElementType elementType, size_type offset)
				:
				_elementType(elementType),
				_offset(offset)
			{


			}

			size_type GetOffsetAfter() const {
				return _offset + Size();
			}

			size_type GetOffset() const {
				return _offset;
			}

			size_type Size() const
			{
				return sizeOf(_elementType);
			}

			const size_type sizeOf(ElementType type) const
			{
				using namespace DirectX;
				switch (type)
				{
				case Position3D:
				case Float3Color:
				case Normal:
					return sizeof(XMFLOAT3);
				case Position2D:
				case Texture2D:
					return sizeof(XMFLOAT2);
				case Float4Color:
					return sizeof(XMFLOAT4);
				case BGRAColor:
					return sizeof(unsigned int);
					break;
				}
				assert("Incorrect Element Type" && false);
				return 0;
			}

			const ElementType GetType() const
			{
				return _elementType;
			}
		};

	private:
		std::vector<Element> _elements;

	public:

		template<ElementType elementType>
		VertexLayout& Append()
		{
			Element::size_type nextOffset = _elements.size() == 0 ? 0 : _elements.back().GetOffsetAfter();
			_elements.emplace_back(elementType, nextOffset);
			return *this;
		}

		template<ElementType elementType>
		Element& Resolve()
		{
			for (auto& e : _elements)
			{
				if (elementType == e.GetType())
				{
					return e;
				}
			}
			assert("Could not resolve element type" && false);
			return _elements.front();
		}

		Element& ResolveByIndex(size_t i)
		{
			return _elements[i];
		}

		Element::size_type Size() const
		{
			return _elements.empty() ? 0 : _elements.back().GetOffsetAfter();
		}

		const std::vector<Element>& getElements() const
		{
			return _elements;
		}
	};



	class Vertex
	{
	private:
		std::uint8_t* _pdata = nullptr;
		VertexLayout& _layout;
		friend class VertexBuffer;

	public:
		Vertex(VertexLayout& layout, std::uint8_t* data) : _pdata(data), _layout(layout) { assert(data != nullptr); }

		template<VertexLayout::ElementType elementType>
		auto& Attr()
		{
			using namespace DirectX;
			VertexLayout::Element& elem = _layout.Resolve<elementType>();
			std::uint8_t* pattr = elem.GetOffset() + _pdata;
			if constexpr (elementType == VertexLayout::Position3D ||
				elementType == VertexLayout::Normal ||
				elementType == VertexLayout::Float3Color)
			{
				return *reinterpret_cast<XMFLOAT3*>(pattr);
			}
			else if constexpr (elementType == VertexLayout::Position2D ||
				elementType == VertexLayout::Texture2D)
			{
				return *reinterpret_cast<XMFLOAT2*>(pattr);
			}
			else if constexpr (elementType == VertexLayout::Float4Color)
			{
				return *reinterpret_cast<XMFLOAT4*>(pattr);
			}
			else if constexpr (elementType == VertexLayout::BGRAColor)
			{
				return *reinterpret_cast<unsigned int*>(pattr);
			}
			else {
				assert("Incorrect element type in Attr()" && false);
				return *pattr;
			}
		}

		template<typename T>
		void SetAttributeByIndex(size_t i, T&& val)
		{
			using namespace DirectX;
			const auto& element = _layout.ResolveByIndex(i);
			auto pattr = _pdata + element.GetOffset();
			auto type = element.GetType();
			switch (type)
			{
			case VertexLayout::ElementType::Position3D:
			case VertexLayout::ElementType::Float3Color:
			case VertexLayout::ElementType::Normal:
				SetAttribute<XMFLOAT3>(pattr, std::forward<T>(val));
				break;
			case VertexLayout::ElementType::Position2D:
			case VertexLayout::ElementType::Texture2D:
				SetAttribute<XMFLOAT2>(pattr, std::forward<T>(val));
				break;
			case VertexLayout::ElementType::Float4Color:
				SetAttribute<XMFLOAT4>(pattr, std::forward<T>(val));
				break;
			case VertexLayout::ElementType::BGRAColor:
				SetAttribute<unsigned int*>(pattr, std::forward<T>(val));
				break;
			default:
				assert("SetAttributeByIndex failed with incorrect element type" && false);
			}
		}

		template<typename First, typename ...Rest>
		void SetAttributeByIndex(size_t i, First&& first, Rest&&... rest)
		{
			SetAttributeByIndex(i, std::forward<First>(first));
			SetAttributeByIndex(i + 1, std::forward<Rest>(rest)...);
		}

		template<typename Dest, typename Source>
		void SetAttribute(std::uint8_t* pDest, Source&& val)
		{
			if constexpr (std::is_assignable<Source, Dest>::value)
			{
				*reinterpret_cast<Dest*>(pDest) = val;
			}
			else
			{
				assert("The parameter type Source is not assignable to type Dest." && false);
			}
		}

	};

	class ConstVertex
	{
	private:
		Vertex _vertex;

	public:
		ConstVertex(const Vertex& vertex)
			:
			_vertex(vertex)
		{
		}

		template<VertexLayout::ElementType elementType>
		const auto& Attr() const
		{
			return const_cast<Vertex&>(_vertex).Attr<elementType>();
		}
	};


	class VertexBuffer
	{
	private:
		std::vector<std::uint8_t> _data;
		VertexLayout _layout;

	public:
		VertexBuffer(VertexLayout vertexLayout)
			:
			_layout(vertexLayout)
		{

		}

		const VertexLayout& GetVertexLayout() const
		{
			return _layout;
		}

		size_t Size() const
		{
			return _data.size() / _layout.Size();
		}

		const std::uint8_t* GetData() const
		{
			return _data.data();
		}

		template<typename ...Params>
		void EmplaceBack(Params&&... params)
		{
			_data.resize(_data.size() + _layout.Size());
			Back().SetAttributeByIndex(0u, std::forward<Params>(params)...);
		}

		Vertex Back()
		{
			assert(_data.size() != 0);
			return Vertex(_layout, _data.data() + _data.size() - _layout.Size());
		}

		Vertex Front()
		{
			assert(_data.size() != 0);
			return Vertex(_layout, _data.data());
		}

		Vertex operator[](size_t i)
		{
			assert(i < Size());
			return Vertex(_layout, i * _layout.Size() + _data.data());
		}

		ConstVertex Back() const
		{
			assert(_data.size() != 0);
			return const_cast<VertexBuffer*>(this)->Back();
		}

		ConstVertex Front() const
		{
			assert(_data.size() != 0);
			return const_cast<VertexBuffer*>(this)->Front();
		}

		ConstVertex operator[](size_t i) const
		{
			assert(_data.size() != 0);
			return const_cast<VertexBuffer*>(this)->operator[](i);
		}
	};
}