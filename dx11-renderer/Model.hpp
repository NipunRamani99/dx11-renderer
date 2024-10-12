#pragma once
#include "DrawableBase.hpp"
#include "BindableBase.hpp"
#include "Vertex.h"
#include "AABB.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "AABBVisualisation.hpp"

class Mesh : public DrawableBase<Mesh>
{
private:
	mutable DirectX::XMFLOAT4X4 _transform;
	AABB _aabb;
	AABBVisualisation viz;
public:
	Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bindable>>& bindables, const AABB & aabb = AABB())
		:
		viz(gfx, aabb)
	{
		if (!IsStaticInitialized())
		{
			AddBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		}

		for (auto& pbind : bindables)
		{
			if (auto index = dynamic_cast<IndexBuffer*>(pbind.get()))
			{
				AddIndexBuffer(std::unique_ptr<IndexBuffer>{ index });
				pbind.release();
			}
			else
			{
				AddBind(std::move(pbind));
			}
		}

		AddBind(std::make_unique<TransformCbuf>(gfx, *this));
		_aabb = aabb;
	}

	void Update(float) noexcept
	{

	}


	void Draw(Graphics & gfx, DirectX::XMMATRIX accumulatedTransform) noexcept
	{

		DirectX::XMStoreFloat4x4(&_transform, accumulatedTransform);
		
		DrawableBase::Draw(gfx);
	}

	void DrawAABB(Graphics& gfx, DirectX::XMMATRIX accumulatedTransform) noexcept
	{
		viz.SetTransform(accumulatedTransform);
		viz.Draw(gfx);
	}

	DirectX::XMMATRIX GetTransformXM() const noexcept override
	{
		return DirectX::XMLoadFloat4x4(&_transform);
	}

	void SetAABB(AABB aabb)
	{
		_aabb = aabb;
	}

	const AABB& getAABB() const
	{
		return _aabb;
	}
};

class Node
{
	friend class Model;
private:
	std::vector<Mesh*> _mesh;
	std::vector<std::unique_ptr<Node>> _nodes;
	DirectX::XMFLOAT4X4 _transform;

public:
	
	Node(std::vector<Mesh*> mesh, DirectX::FXMMATRIX & transform)
		:
		_mesh(mesh)
	{
		DirectX::XMStoreFloat4x4(&_transform, transform);
	}

	void AddNode(std::unique_ptr<Node> node)
	{
		_nodes.push_back(std::move(node));
	}

	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform)
	{
		const auto nodeTransform = DirectX::XMLoadFloat4x4(&_transform) * accumulatedTransform;
		for (auto pmesh : _mesh)
		{
			pmesh->Draw(gfx, nodeTransform);
		}
		for (auto & pnode : _nodes)
		{
			pnode->Draw(gfx, nodeTransform);
		}
	}

	void DrawAABB(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform)
	{
		const auto nodeTransform = DirectX::XMLoadFloat4x4(&_transform) * accumulatedTransform;
		for (auto pmesh : _mesh)
		{
			pmesh->DrawAABB(gfx, nodeTransform);
		}
		for (auto& pnode : _nodes)
		{
			pnode->DrawAABB(gfx, nodeTransform);
		}
	}
};

class Model
{
private:
	std::vector<std::unique_ptr<Mesh>> _meshes;
	std::unique_ptr<Node> _root;
	
public:
	Model(Graphics& gfx, const std::string modelPath)
	{
		Assimp::Importer importer;
		const auto pScene = importer.ReadFile(modelPath.c_str(), 
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices );

		for (size_t i = 0; i < pScene->mNumMeshes; i++)
		{
			_meshes.push_back(ParseMesh(gfx, *pScene->mMeshes[i]));
		}
		_root = ParseNode(*pScene->mRootNode);
	}

	std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh)
	{
		namespace dx = DirectX;

		hw3dexp::VertexBuffer vbuf(std::move(
			hw3dexp::VertexLayout{}
			.Append<hw3dexp::VertexLayout::Position3D>()
			.Append<hw3dexp::VertexLayout::Normal>()
		));
		AABB aabb;
		aabb.min.x = mesh.mVertices[0].x;
		aabb.min.y = mesh.mVertices[0].y;
		aabb.min.z = mesh.mVertices[0].z;
		aabb.max.x = mesh.mVertices[0].x;
		aabb.max.y = mesh.mVertices[0].y;
		aabb.max.z = mesh.mVertices[0].z;

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			aabb.min.x = std::min(mesh.mVertices[i].x, aabb.min.x);
			aabb.min.y = std::min(mesh.mVertices[i].y, aabb.min.y);
			aabb.min.z = std::min(mesh.mVertices[i].z, aabb.min.z);
			aabb.max.x = std::max(mesh.mVertices[i].x, aabb.max.x);
			aabb.max.y = std::max(mesh.mVertices[i].y, aabb.max.y);
			aabb.max.z = std::max(mesh.mVertices[i].z, aabb.max.z);

			vbuf.EmplaceBack(
				dx::XMFLOAT3{ mesh.mVertices[i].x, mesh.mVertices[i].y , mesh.mVertices[i].z },
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i])
			);
		}

		std::vector<unsigned int> indices;
		indices.reserve(mesh.mNumFaces * 3);

		for (unsigned int i = 0; i < mesh.mNumFaces; i++)
		{
			const auto& face = mesh.mFaces[i];
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}

		std::vector<std::unique_ptr<Bindable>> bindables;
		bindables.push_back(std::make_unique<VertexBuffer>(gfx, vbuf));
		
		bindables.push_back(std::make_unique<IndexBuffer>(gfx, indices));

		auto pvs = std::make_unique<VertexShader>(gfx, L"PhongVS.cso");
		auto pvsbc = pvs->GetBytecode();

		bindables.push_back(std::move(pvs));

		bindables.push_back(std::make_unique<PixelShader>(gfx, L"PhongPS.cso"));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		bindables.push_back(std::make_unique<InputLayout>(gfx, ied, pvsbc));

		struct ObjectData {
			alignas(16) dx::XMFLOAT3 material;
			float specularIntensity = 0.60f;
			float specularPower = 30.0f;
		} objectData;
		objectData.material = {1.0f, 0.2f, 0.1f};
		 bindables.push_back(std::make_unique<PixelConstantBuffer<ObjectData>>(gfx, objectData, 1));

		return make_unique<Mesh>(gfx, bindables, aabb);
	}

	DirectX::XMMATRIX ConvertToMatrix(const aiMatrix4x4& mat) {
		return DirectX::XMMATRIX(
			mat.a1, mat.a2, mat.a3, mat.a4,
			mat.b1, mat.b2, mat.b3, mat.b4,
			mat.c1, mat.c2, mat.c3, mat.c4,
			mat.d1, mat.d2, mat.d3, mat.d4
		);
	}

	std::unique_ptr<Node> ParseNode(const aiNode& node)
	{
		std::vector<Mesh*> meshes;

		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			meshes.push_back( _meshes[node.mMeshes[i]].get());
		}
		std::string name = node.mName.C_Str();;
		DirectX::XMMATRIX matrix = DirectX::XMMatrixTranspose(
			DirectX::XMLoadFloat4x4( 
				reinterpret_cast<const DirectX::XMFLOAT4X4*>(&node.mTransformation)
			)
		);
		
		std::unique_ptr<Node> pNode = std::make_unique<Node>(meshes, matrix);
		
		for (size_t i = 0; i < node.mNumChildren; i++)
		{
			std::unique_ptr<Node> ch = ParseNode(*node.mChildren[i]);
			pNode->AddNode(std::move(ch));
		}
		return pNode;
	}

	void Draw(Graphics& gfx)
	{
		_root->Draw(gfx, DirectX::XMMatrixIdentity());
	}

	void DrawAABB(Graphics& gfx)
	{
		_root->DrawAABB(gfx, DirectX::XMMatrixIdentity());
	}
};