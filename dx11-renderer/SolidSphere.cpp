#include "SolidSphere.hpp"
#include "BindableBase.hpp"
#include "GraphicsThrowMacros.h"
#include "Sphere.hpp"


SolidSphere::SolidSphere(Graphics& gfx, float radius)
{
	namespace dx = DirectX;
	using namespace Bind;
	struct Vertex
	{
		dx::XMFLOAT3 pos;
	};
	auto model = Sphere::Make<Vertex>();
	model.Transform(dx::XMMatrixScaling(radius, radius, radius));
	AddBind(std::make_shared<VertexBuffer>(gfx, model.vertices));
	AddBind(std::make_shared<IndexBuffer>(gfx, model.indices));

	auto pvs = std::make_unique<VertexShader>(gfx, L"SolidVS.cso");
	auto pvsbc = pvs->GetBytecode();
	AddBind(std::move(pvs));

	AddBind(std::make_unique<PixelShader>(gfx, L"SolidPS.cso"));

	struct PSColorConstant
	{
		dx::XMFLOAT3 color = { 1.0f,1.0f,1.0f };
		float padding;
	} colorConst;
	AddBind(std::make_unique<PixelConstantBuffer<PSColorConstant>>(gfx, colorConst));

	const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
	{
		{ "Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
	};
	AddBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));

	AddBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	AddBind(std::make_unique<TransformCbuf>(gfx, *this));
}

void SolidSphere::SetPos(DirectX::XMFLOAT3 pos) noexcept
{
	this->_pos = pos;
}

DirectX::XMMATRIX SolidSphere::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixTranslation(_pos.x, _pos.y, _pos.z);
}
