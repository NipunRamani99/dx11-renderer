#include "BindableBase.hpp"
#include "AssimpTest.hpp"
#include "GraphicsThrowMacros.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>
AssimpTest::AssimpTest(Graphics& gfx, 
	std::mt19937& rng, 
	std::uniform_real_distribution<float>& adist, 
	std::uniform_real_distribution<float>& ddist, 
	std::uniform_real_distribution<float>& odist, 
	std::uniform_real_distribution<float>& rdist, 
	DirectX::XMFLOAT3 material,
	float scale)
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
	material(material),
	scale(scale)
{
	namespace dx = DirectX;
	if (!IsStaticInitialized()) 
	{
		BindForPhongShader(gfx, scale);
	} 
	else
	{
		SetIndexFromStatic();
	}

	dx::XMStoreFloat3x3(
		&mt,
		dx::XMMatrixScaling(1.0f, 1.0f, 1.0f)
	);
	AddBind(std::make_unique<TransformCbuf>(gfx, *this));
}

void AssimpTest::Update(float dt) noexcept
{
	roll += droll * dt;
	pitch += dpitch * dt;
	yaw += dyaw * dt;
	theta += dtheta * dt;
	phi += dphi * dt;
	chi += dchi * dt;
}

DirectX::XMMATRIX AssimpTest::GetTransformXM() const noexcept
{
	namespace dx = DirectX;
	return dx::XMLoadFloat3x3(&mt) *
		dx::XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
		dx::XMMatrixTranslation(r, 0.0f, 0.0f) *
		dx::XMMatrixRotationRollPitchYaw(theta, phi, chi);
}

void AssimpTest::BindForPhongShader(Graphics& gfx, float scale)
{
	namespace dx = DirectX;

	struct Vertex
	{
		dx::XMFLOAT3 pos;
		dx::XMFLOAT3 n;
	};

	Assimp::Importer importer;
	const auto pmodel = importer.ReadFile("./Models/suzanne.obj", aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
	const auto pmesh = pmodel->mMeshes[0];

	std::vector<Vertex> vertices;
	vertices.reserve(pmesh->mNumVertices);
	for (unsigned int i = 0; i < pmesh->mNumVertices; i++)
	{
		vertices.push_back({
			{ pmesh->mVertices[i].x*scale, pmesh->mVertices[i].y * scale, pmesh->mVertices[i].z * scale },
			*reinterpret_cast<dx::XMFLOAT3*>(&pmesh->mNormals[i])
			});
	}

	std::vector<unsigned short> indices;
	indices.reserve(pmesh->mNumFaces * 3);
	
	for (unsigned int i = 0; i < pmesh->mNumFaces; i++) {
		const auto& face = pmesh->mFaces[i];
		assert(face.mNumIndices == 3);
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}

	AddStaticBind(std::make_unique<VertexBuffer>(gfx, vertices));

	AddStaticIndexBuffer(std::make_unique<IndexBuffer>(gfx, indices));

	auto pvs = std::make_unique<VertexShader>(gfx, L"PhongVS.cso");
	auto pvsbc = pvs->GetBytecode();
	AddStaticBind(std::move(pvs));

	AddStaticBind(std::make_unique<PixelShader>(gfx, L"PhongPS.cso"));

	const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
	{
		{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));

	AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));


	struct PSMaterialConstant
	{
		DirectX::XMFLOAT3 color;
		float specularIntensity = 0.6f;
		float specularPower = 30.0f;
		float padding[3];
	} pmc;
	pmc.color = material;
	AddStaticBind(std::make_unique<PixelConstantBuffer<PSMaterialConstant>>(gfx, pmc, 1u));

}