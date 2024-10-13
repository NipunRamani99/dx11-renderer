#include "Mesh.hpp"

Mesh::Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bind::Bindable>>& bindables, const AABB& aabb)
	:
	viz(gfx, aabb)
{
	if (!IsStaticInitialized())
	{
		AddBind(std::make_unique<Bind::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}

	for (auto& pbind : bindables)
	{
		if (auto index = dynamic_cast<Bind::IndexBuffer*>(pbind.get()))
		{
			AddIndexBuffer(std::unique_ptr<Bind::IndexBuffer>{ index });
			pbind.release();
		}
		else
		{
			AddBind(std::move(pbind));
		}
	}

	AddBind(std::make_unique<Bind::TransformCbuf>(gfx, *this));
	_aabb = aabb;
}

void Mesh::Update(float) noexcept
{

}

void Mesh::Draw(Graphics& gfx, DirectX::XMMATRIX accumulatedTransform) noexcept
{

	DirectX::XMStoreFloat4x4(&_transform, accumulatedTransform);

	DrawableBase::Draw(gfx);
}

void Mesh::DrawAABB(Graphics& gfx, DirectX::XMMATRIX accumulatedTransform) noexcept
{
	viz.SetTransform(accumulatedTransform);
	viz.Draw(gfx);
}

DirectX::XMMATRIX Mesh::GetTransformXM() const noexcept
{
	return DirectX::XMLoadFloat4x4(&_transform);
}

void Mesh::SetAABB(AABB aabb)
{
	_aabb = aabb;
}

const AABB& Mesh::getAABB() const
{
	return _aabb;
}

Node::Node(std::vector<Mesh*> mesh, DirectX::FXMMATRIX& transform)
	:
	_mesh(mesh)
{
	DirectX::XMStoreFloat4x4(&_transform, transform);
}

void Node::AddNode(std::unique_ptr<Node> node)
{
	_nodes.push_back(std::move(node));
}

void Node::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform)
{
	const auto nodeTransform = DirectX::XMLoadFloat4x4(&_transform) * accumulatedTransform;
	for (auto pmesh : _mesh)
	{
		pmesh->Draw(gfx, nodeTransform);
	}
	for (auto& pnode : _nodes)
	{
		pnode->Draw(gfx, nodeTransform);
	}
}

void Node::DrawAABB(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform)
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

Model::Model(Graphics& gfx, const std::string modelPath)
{
	Assimp::Importer importer;
	const auto pScene = importer.ReadFile(modelPath.c_str(),
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices);

	for (size_t i = 0; i < pScene->mNumMeshes; i++)
	{
		_meshes.push_back(ParseMesh(gfx, *pScene->mMeshes[i]));
	}
	_root = ParseNode(*pScene->mRootNode);
}

void Model::Draw(Graphics& gfx)
{
	_root->Draw(gfx, DirectX::XMMatrixIdentity());
}

void Model::DrawAABB(Graphics& gfx)
{
	_root->DrawAABB(gfx, DirectX::XMMatrixIdentity());
}

std::unique_ptr<Mesh> Model::ParseMesh(Graphics& gfx, const aiMesh& mesh)
{
	namespace dx = DirectX;

	Dvtx::VertexBuffer vbuf(std::move(
		Dvtx::VertexLayout{}
		.Append<Dvtx::VertexLayout::Position3D>()
		.Append<Dvtx::VertexLayout::Normal>()
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

	std::vector<std::unique_ptr<Bind::Bindable>> bindables;
	bindables.push_back(std::make_unique<Bind::VertexBuffer>(gfx, vbuf));

	bindables.push_back(std::make_unique<Bind::IndexBuffer>(gfx, indices));

	auto pvs = std::make_unique<Bind::VertexShader>(gfx, L"PhongVS.cso");
	auto pvsbc = pvs->GetBytecode();

	bindables.push_back(std::move(pvs));

	bindables.push_back(std::make_unique<Bind::PixelShader>(gfx, L"PhongPS.cso"));

	const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
	{
		{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	bindables.push_back(std::make_unique<Bind::InputLayout>(gfx, ied, pvsbc));

	struct ObjectData {
		alignas(16) dx::XMFLOAT3 material;
		float specularIntensity = 0.60f;
		float specularPower = 30.0f;
	} objectData;
	objectData.material = { 1.0f, 0.2f, 0.1f };
	bindables.push_back(std::make_unique<Bind::PixelConstantBuffer<ObjectData>>(gfx, objectData, 1));

	return make_unique<Mesh>(gfx, bindables, aabb);
}

DirectX::XMMATRIX Model::ConvertToMatrix(const aiMatrix4x4& mat) {
	return DirectX::XMMATRIX(
		mat.a1, mat.a2, mat.a3, mat.a4,
		mat.b1, mat.b2, mat.b3, mat.b4,
		mat.c1, mat.c2, mat.c3, mat.c4,
		mat.d1, mat.d2, mat.d3, mat.d4
	);
}

std::unique_ptr<Node> Model::ParseNode(const aiNode& node)
{
	std::vector<Mesh*> meshes;

	for (size_t i = 0; i < node.mNumMeshes; i++)
	{
		meshes.push_back(_meshes[node.mMeshes[i]].get());
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
