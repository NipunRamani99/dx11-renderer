#define TINYBVH_IMPLEMENTATION
#include "Mesh.hpp"
#include "imgui\imgui.h"
#include <unordered_map>

Mesh::Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bind::Bindable>>& bindables, std::unique_ptr<tinybvh::BVH> bvh, std::vector<tinybvh::bvhvec4> & vertices, const AABB& aabb)
	:
	Mesh(gfx,bindables,aabb)
{
	this->bvh = std::move(bvh);
	this->vertices = std::move(vertices);
}

Mesh::Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bind::Bindable>>& bindables, const AABB& aabb)
	:
	viz(gfx, aabb)
{
	using namespace Bind;

	if (!IsStaticInitialized())
	{
		AddBind(std::make_unique<Bind::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
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

void Mesh::SetBVH(std::unique_ptr<tinybvh::BVH> bvh)
{
	this->bvh = std::move(bvh);
}

const tinybvh::BVH & Mesh::GetBVH()
{
	return *bvh.get();
}

Node::Node(std::string name, std::vector<Mesh*> mesh, DirectX::FXMMATRIX& transform)
	:
	_mesh(mesh),
	_name(name)
{
	DirectX::XMStoreFloat4x4(&_basetransform, transform);
	DirectX::XMStoreFloat4x4(&_appliedtransform, DirectX::XMMatrixIdentity());
}

void Node::AddNode(std::unique_ptr<Node> node)
{
	_nodes.push_back(std::move(node));
}

void Node::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform = DirectX::XMMatrixIdentity())
{
	const auto nodeTransform = DirectX::XMLoadFloat4x4(&_appliedtransform) * DirectX::XMLoadFloat4x4(&_basetransform) * accumulatedTransform;
	for (auto pmesh : _mesh)
	{
		pmesh->Draw(gfx, nodeTransform);
	}
	for (auto& pnode : _nodes)
	{
		pnode->Draw(gfx, nodeTransform);
	}
}

void Node::DrawAABB(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform = DirectX::XMMatrixIdentity())
{
	const auto nodeTransform = DirectX::XMLoadFloat4x4(&_appliedtransform) * DirectX::XMLoadFloat4x4(&_basetransform) * accumulatedTransform;
	for (auto pmesh : _mesh)
	{
		pmesh->DrawAABB(gfx, nodeTransform);
	}
	for (auto& pnode : _nodes)
	{
		pnode->DrawAABB(gfx, nodeTransform);
	}
}

void Node::ShowWindow(int & nodeIndex, std::optional<int>& selectedIndex, Node *& selectedNode) const
{
	int currentNodeIndex = nodeIndex;
	nodeIndex++;
	ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_OpenOnArrow |
		(selectedIndex.value_or(-1) == currentNodeIndex ? ImGuiTreeNodeFlags_Selected : 0 |
			_nodes.empty() ? ImGuiTreeNodeFlags_Leaf : 0);
	if (ImGui::TreeNodeEx((void*)(intptr_t)currentNodeIndex, flag, _name.c_str()))
	{
		selectedIndex = ImGui::IsItemClicked() ? currentNodeIndex : selectedIndex;
		selectedNode = ImGui::IsItemClicked() ? const_cast<Node*>(this) : selectedNode;
		for (auto& pnode : _nodes)
		{
			pnode->ShowWindow(nodeIndex, selectedIndex, selectedNode);
		}
		ImGui::TreePop();
	}
}

void Node::SetAppliedTransform(DirectX::FXMMATRIX appliedTransform)
{
	DirectX::XMStoreFloat4x4(&_appliedtransform, appliedTransform);
}

void Node::IntersectNode(const DirectX::XMMATRIX& accumulatedTransform, const tinybvh::Ray& rayWorld, IntersectionResult& closestHit) {
	// Compute the node's world transformation matrix
	DirectX::XMMATRIX nodeTransform = DirectX::XMLoadFloat4x4(&_appliedtransform) * DirectX::XMLoadFloat4x4(&_basetransform) * accumulatedTransform;

	// For each Mesh in the Node
	for (const auto& pmesh : _mesh) {
		// Get the inverse of the Mesh's world transform
		DirectX::XMMATRIX invTransform = DirectX::XMMatrixInverse(nullptr, nodeTransform);

		// Transform the ray into the Mesh's local space
		DirectX::XMVECTOR rayOriginWorld = DirectX::XMVectorSet(rayWorld.O.x, rayWorld.O.y, rayWorld.O.z, 1.0f);
		DirectX::XMVECTOR rayOriginLocal = DirectX::XMVector3Transform(rayOriginWorld, invTransform);

		DirectX::XMVECTOR rayDirWorld = DirectX::XMVectorSet(rayWorld.D.x, rayWorld.D.y, rayWorld.D.z, 0.0f);
		DirectX::XMVECTOR rayDirLocal = DirectX::XMVector3TransformNormal(rayDirWorld, invTransform);
		rayDirLocal = DirectX::XMVector3Normalize(rayDirLocal);

		// Convert to tinybvh::Ray
		tinybvh::bvhvec3 rayOriginLocalVec;
		tinybvh::bvhvec3 rayDirLocalVec;
		DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&rayOriginLocalVec), rayOriginLocal);
		DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&rayDirLocalVec), rayDirLocal);

		tinybvh::Ray rayLocal(rayOriginLocalVec, rayDirLocalVec);

		// Perform intersection with the Mesh's BVH
		const tinybvh::BVH& bvh = pmesh->GetBVH();
		bvh.Intersect(rayLocal);

		if (rayLocal.hit.t < closestHit.t && rayLocal.hit.t > 0) {
			// Update closest hit
			closestHit.hit = true;
			closestHit.t = rayLocal.hit.t;

			// Compute intersection point in local space
			tinybvh::bvhvec3 hitPointLocal = rayLocal.O + rayLocal.D * rayLocal.hit.t;

			// Transform the hit point back to world space
			DirectX::XMVECTOR hitPointLocalVec = DirectX::XMVectorSet(hitPointLocal.x, hitPointLocal.y, hitPointLocal.z, 1.0f);
			DirectX::XMVECTOR hitPointWorldVec = DirectX::XMVector3Transform(hitPointLocalVec, nodeTransform);

			DirectX::XMFLOAT3 hitPointWorld;
			DirectX::XMStoreFloat3(&hitPointWorld, hitPointWorldVec);

			closestHit.point = tinybvh::bvhvec3(hitPointWorld.x, hitPointWorld.y, hitPointWorld.z);
			closestHit.mesh = pmesh;
			closestHit.node = this;
			closestHit.triangleIndex = rayLocal.hit.prim;
		}
	}

	// Recurse into child Nodes
	for (const auto & childNode : _nodes) {
		childNode->IntersectNode(nodeTransform, rayWorld, closestHit);
	}
}

std::string Node::GetName() const
{
	return _name;
}

class ModelWindow
{
public:
	ModelWindow() {}
	void ShowWindow(std::string name, const Node & rootNode)
	{
		ImGui::Begin("Model");
		ImGui::Columns(2, nullptr, true);
		int nodeIndex = 0;
		if (ImGui::TreeNodeEx(name.c_str()))
		{

			rootNode.ShowWindow(nodeIndex, selectedIndex, _pselectednode);
			ImGui::TreePop();

		}
		ImGui::NextColumn();
		int index = selectedIndex.value();
		ImGui::SliderAngle("Yaw", (float*)&transforms[index].yaw, -180.0f, 180.0f);
		ImGui::SliderAngle("Pitch", (float*)&transforms[index].pitch, -180.0f, 180.0f);
		ImGui::SliderAngle("Roll", (float*)&transforms[index].roll, -180.0f, 180.0f);
		ImGui::InputFloat("Position X", (float*)&transforms[index].x, 0.1f, 1.0f, "%.3f");
		ImGui::InputFloat("Position Y", (float*)&transforms[index].y, 0.1f, 1.0f, "%.3f");
		ImGui::InputFloat("Position Z", (float*)&transforms[index].z, 0.1f, 1.0f, "%.3f");
		ImGui::Text("Selected Index: %d", selectedIndex.value());
		ImGui::End();
	}

	DirectX::XMMATRIX GetTransformation()
	{
		DirectX::XMMATRIX appliedTransfrom = DirectX::XMMatrixIdentity();
		if (selectedIndex.value() > -1)
		{
			auto transform = transforms[selectedIndex.value()];
			appliedTransfrom = DirectX::XMMatrixRotationRollPitchYaw(transform.pitch, transform.yaw, transform.roll)
				* DirectX::XMMatrixTranslation(transform.x, transform.y, transform.z);

		}
		return appliedTransfrom;
	}

	Node* GetSelectedNode()
	{
		return _pselectednode;
	}

	~ModelWindow() = default;
private:
	std::optional<int> selectedIndex = -1;
	struct Pos
	{
		float yaw = 0.0f;
		float roll = 0.0f;
		float pitch = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	};
	std::unordered_map<int, Pos> transforms;
	Node* _pselectednode = nullptr;
};

Model::Model(Graphics& gfx, const std::string modelPath)
	
{
	Assimp::Importer importer;
	const auto pScene = importer.ReadFile(modelPath.c_str(),
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices);

	_name = pScene->mName.C_Str();
	_name = _name.empty() ? "Sample Scene" : _name;
	for (size_t i = 0; i < pScene->mNumMeshes; i++)
	{
		_meshes.push_back(ParseMesh(gfx, *pScene->mMeshes[i]));
	}
	_root = ParseNode(*pScene->mRootNode);
	_pwindow = std::make_unique<ModelWindow>();
}

void Model::Draw(Graphics& gfx)
{
	if (auto node = _pwindow->GetSelectedNode())
	{
		DirectX::XMMATRIX transform = _pwindow->GetTransformation();
		node->SetAppliedTransform(transform);
	}
	_root->Draw(gfx);
}

void Model::DrawAABB(Graphics& gfx)
{
	_root->DrawAABB(gfx);
}

std::unique_ptr<Mesh> Model::ParseMesh(Graphics& gfx, const aiMesh& mesh)
{
	namespace dx = DirectX;
	using namespace Bind;

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
	std::vector<tinybvh::bvhvec4> vertices;
	indices.reserve(mesh.mNumFaces * 3);
	vertices.reserve(mesh.mNumFaces * 3);
	for (unsigned int i = 0; i < mesh.mNumFaces; i++)
	{
		const auto& face = mesh.mFaces[i];
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
		auto v1 = mesh.mVertices[face.mIndices[0]];
		auto v2 = mesh.mVertices[face.mIndices[1]];
		auto v3 = mesh.mVertices[face.mIndices[2]];

		vertices.push_back({ v1.x, v1.y, v1.z, 0 });
		vertices.push_back({ v2.x, v2.y, v2.z ,0 });
		vertices.push_back({ v3.x, v3.y, v3.z ,0 });
	}
	
	std::unique_ptr<tinybvh::BVH> bvh = std::make_unique<tinybvh::BVH>();
	bvh->Build(vertices.data(), mesh.mNumFaces);
	std::vector<std::unique_ptr<Bindable>> bindables;
	bindables.push_back(std::make_unique<VertexBuffer>(gfx, vbuf));

	bindables.push_back(std::make_unique<IndexBuffer>(gfx, indices));

	auto pvs = std::make_unique<VertexShader>(gfx, L"PhongVS.cso");
	auto pvsbc = pvs->GetBytecode();

	bindables.push_back(std::move(pvs));

	bindables.push_back(std::make_unique<PixelShader>(gfx, L"PhongPS.cso"));

	const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
	{
		{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	bindables.push_back(std::make_unique<InputLayout>(gfx, ied, pvsbc));

	struct ObjectData {
		alignas(16) dx::XMFLOAT3 material;
		float specularIntensity = 0.60f;
		float specularPower = 30.0f;
	} objectData;
	objectData.material = { 1.0f, 0.2f, 0.1f };
	bindables.push_back(std::make_unique<PixelConstantBuffer<ObjectData>>(gfx, objectData, 1));

	return make_unique<Mesh>(gfx, bindables, std::move(bvh), vertices, aabb);
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
	std::string name = node.mName.C_Str();
	name = name.empty() ? "Placeholder" : name;
	DirectX::XMMATRIX matrix = DirectX::XMMatrixTranspose(
		DirectX::XMLoadFloat4x4(
			reinterpret_cast<const DirectX::XMFLOAT4X4*>(&node.mTransformation)
		)
	);

	std::unique_ptr<Node> pNode = std::make_unique<Node>(name, meshes, matrix);

	for (size_t i = 0; i < node.mNumChildren; i++)
	{
		std::unique_ptr<Node> ch = ParseNode(*node.mChildren[i]);
		pNode->AddNode(std::move(ch));
	}
	return pNode;
}

void Model::ShowWindow()
{
	_pwindow->ShowWindow(_name, *_root);
}

IntersectionResult Model::IntersectMesh(const DirectX::XMFLOAT3 rayOriginWorld, const DirectX::XMFLOAT3 rayDirectionWorld)
{
	tinybvh::bvhvec3& rayOrigin = *reinterpret_cast<tinybvh::bvhvec3*>(const_cast<DirectX::XMFLOAT3*>(&rayOriginWorld));
	tinybvh::bvhvec3& rayDirection = *reinterpret_cast<tinybvh::bvhvec3*>(const_cast<DirectX::XMFLOAT3*>(&rayDirectionWorld));
	tinybvh::Ray ray{rayOrigin, rayDirection};
	IntersectionResult result;
	if (_root)
	{
		_root->IntersectNode(DirectX::XMMatrixIdentity(), ray, result);
	}
	return result;
}

Model::~Model() = default;