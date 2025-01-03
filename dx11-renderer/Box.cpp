#include "Box.hpp"
#include "BindableBase.hpp"
#include "GraphicsThrowMacros.h"
#include "Cube.hpp"

using namespace Bind;

Box::Box(Graphics& gfx,
	std::mt19937& rng,
	std::uniform_real_distribution<float>& adist,
	std::uniform_real_distribution<float>& ddist,
	std::uniform_real_distribution<float>& odist,
	std::uniform_real_distribution<float>& rdist,
	std::uniform_real_distribution<float>& bdist,
	DirectX::XMFLOAT3 mat)
	:
	r(rdist(rng)),
	droll(ddist(rng)),
	dpitch(ddist(rng)),
	dyaw(ddist(rng)),
	dphi(odist(rng)),
	dtheta(odist(rng)),
	dchi(odist(rng)),
	chi(adist(rng)),
	theta(adist(rng)),
	phi(adist(rng)),
	material(mat)
{
	namespace dx = DirectX;

	BindForPhongShader(gfx);


	AddBind(std::make_unique<TransformCbuf>(gfx, *this));

	// model deformation transform (per instance, not stored as bind)
	dx::XMStoreFloat3x3(
		&mt,
		dx::XMMatrixScaling(1.0f, 1.0f, bdist(rng))
	);
	struct ObjectData {
		alignas(16) dx::XMFLOAT3 material;
		float specularIntensity = 0.60f;
		float specularPower = 30.0f;
	} objectData;
	objectData.material = material;
	AddBind(std::make_unique<PixelConstantBuffer<ObjectData>>(gfx, objectData, 1));
}

void Box::Update(float dt) noexcept
{
	roll += droll * dt;
	pitch += dpitch * dt;
	yaw += dyaw * dt;
	theta += dtheta * dt;
	phi += dphi * dt;
	chi += dchi * dt;
}

DirectX::XMMATRIX Box::GetTransformXM() const noexcept
{
	namespace dx = DirectX;
	return dx::XMLoadFloat3x3(&mt) *
		dx::XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
		dx::XMMatrixTranslation(r, 0.0f, 0.0f) *
		dx::XMMatrixRotationRollPitchYaw(theta, phi, chi);
}

void Box::BindForToonShader(Graphics& gfx)
{
	namespace dx = DirectX;

	struct Vertex
	{
		dx::XMFLOAT3 pos;
		dx::XMFLOAT3 n;
	};

	auto model = Cube::MakeIndependent<Vertex>();
	model.SetNormalsIndependentFlat();

	AddBind(std::make_unique<VertexBuffer>(gfx, model.vertices));

	auto pvs = std::make_unique<VertexShader>(gfx, L"ToonVS.cso");
	auto pvsbc = pvs->GetBytecode();
	AddBind(std::move(pvs));

	auto pfs = std::make_unique<PixelShader>(gfx, L"ToonPS.cso");
	AddBind(std::move(pfs));
	
	AddBind(std::make_unique<IndexBuffer>(gfx, model.indices));

	const std::vector<D3D11_INPUT_ELEMENT_DESC> ied = {
		{ "Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "Normal", 0,DXGI_FORMAT_R32G32B32_FLOAT, 0,12,D3D11_INPUT_PER_VERTEX_DATA,0 },
	};

	AddBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));

	AddBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
}

void Box::BindForPhongShader(Graphics & gfx) 
{
	namespace dx = DirectX;

	struct Vertex
	{
		dx::XMFLOAT3 pos;
		dx::XMFLOAT3 n;
	};
	auto model = Cube::MakeIndependent<Vertex>();
	model.SetNormalsIndependentFlat();

	AddBind(std::make_unique<VertexBuffer>(gfx, model.vertices));

	auto pvs = std::make_unique<VertexShader>(gfx, L"PhongVS.cso");
	auto pvsbc = pvs->GetBytecode();
	AddBind(std::move(pvs));

	AddBind(std::make_unique<PixelShader>(gfx, L"PhongPS.cso"));

	AddBind(std::make_unique<IndexBuffer>(gfx, model.indices));

	const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
	{
		{ "Position",0, DXGI_FORMAT_R32G32B32_FLOAT,0, 0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 },
	};

	AddBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));

	AddBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
}