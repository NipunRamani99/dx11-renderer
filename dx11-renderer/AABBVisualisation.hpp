#pragma once
#include "DrawableBase.hpp"
#include "AABB.hpp"
#include "IndexBuffer.hpp"
#include "VertexShader.hpp"
#include "PixelShader.hpp"
#include "ConstantBuffers.hpp"
#include "TransformCbuf.hpp"
#include "Vertex.h"
#include "Cube.hpp"
class AABBVisualisation : public DrawableBase<AABBVisualisation>
{
private:
	AABB _aabb;
	DirectX::XMMATRIX _transform;
	float _scale_x = 0.0f;
	float _scale_y = 0.0f;
	float _scale_z = 0.0f;
public:
	AABBVisualisation(Graphics & gfx, const AABB & aabb)
		:
		_aabb(aabb),
		_transform(DirectX::XMMatrixIdentity()),
		_scale_x(aabb.max.x - aabb.min.x),
		_scale_y(aabb.max.y - aabb.min.y),
		_scale_z(aabb.max.z - aabb.min.z)
	{
		Dvtx::VertexLayout layout;
		layout.Append<Dvtx::VertexLayout::ElementType::Position3D>();
		Dvtx::VertexBuffer buf{ layout };
		struct Vertex
		{
			DirectX::XMFLOAT3 pos;
		};
		IndexedTriangleList<Vertex> cube = Cube::MakeWireframe<Vertex>(aabb);
		for (size_t i = 0; i < cube.vertices.size(); i++)
		{
			buf.EmplaceBack(cube.vertices[i].pos);
		}

		using namespace Bind;
		
		AddBind(std::make_unique<VertexBuffer>(gfx, buf));

		AddIndexBuffer(std::make_unique<IndexBuffer>(gfx, cube.indices));

		auto vs = std::make_unique<VertexShader>(gfx, L"SolidVS.cso");
		auto vsbc = vs->GetBytecode();

		AddBind(std::move(vs));
		
		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied = {
			{ "Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		};

		AddBind(std::make_unique<InputLayout>(gfx, ied, vsbc));
		
		AddBind(std::make_unique<PixelShader>(gfx, L"SolidPS.cso"));

		AddBind(std::make_unique<TransformCbuf>(gfx, *this));

		AddBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST));
	}

	void SetTransform(const DirectX::XMFLOAT4X4& transform)
	{
		_transform = DirectX::XMLoadFloat4x4(&transform);
	}

	void SetTransform(const DirectX::XMMATRIX& transform)
	{
		_transform = transform;
	}

	void SetAABB(const AABB aabb)
	{
		_aabb = aabb;
		_scale_x = aabb.max.x - aabb.min.x;
		_scale_y = aabb.max.y - aabb.min.y;
		_scale_z = aabb.max.z - aabb.min.z;
	}

	DirectX::XMMATRIX GetTransformXM() const noexcept
	{
		return _transform;
	}

	void Update(float) noexcept
	{
	}
};