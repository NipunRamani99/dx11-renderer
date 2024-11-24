#pragma once
#include "BindableBase.hpp"
#include "Vertex.h"
#include "AABB.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "AABBVisualisation.hpp"
#include "tiny_bvh\tiny_bvh.h"
#include <memory>
#include <optional>

class Mesh : public Drawable
{
private:
	mutable DirectX::XMFLOAT4X4 _transform;
	AABB _aabb;
	AABBVisualisation viz;
	std::unique_ptr<tinybvh::BVH> bvh;
	std::vector<tinybvh::bvhvec4> vertices;

public:
	Mesh(Graphics& gfx, std::vector<std::shared_ptr<Bind::Bindable>>& bindables,  
		std::unique_ptr<tinybvh::BVH> bvh, std::vector<tinybvh::bvhvec4> & vertices, 
		const AABB& aabb = AABB(), 
		std::string name = "");
	Mesh(Graphics& gfx, std::vector<std::shared_ptr<Bind::Bindable>>& bindables, 
		const AABB & aabb = AABB(), 
		std::string name= "");
	void Update(float) noexcept;
	void Draw(Graphics & gfx, DirectX::XMMATRIX accumulatedTransform) noexcept;
	void DrawAABB(Graphics& gfx, DirectX::XMMATRIX accumulatedTransform) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void SetAABB(AABB aabb);
	const AABB& getAABB() const;
	void SetBVH(std::unique_ptr<tinybvh::BVH> bvh);
	const tinybvh::BVH& GetBVH();
};

class Node;

// Define a struct to hold intersection details
struct IntersectionResult {
	bool hit = false;
	float t = 1e30f; // Initialize to max float value
	tinybvh::bvhvec3 point; // Intersection point in world space
	Mesh* mesh = nullptr; // Pointer to the Mesh that was hit
	Node* node = nullptr; // Pointer to the Node that was hit
	unsigned int triangleIndex = 0; // Index of the triangle hit
};


class Node
{
	friend class Model;
private:
	std::vector<Mesh*> _mesh;
	std::string _name;
	std::vector<std::unique_ptr<Node>> _nodes;
	DirectX::XMFLOAT4X4 _basetransform;
	DirectX::XMFLOAT4X4 _appliedtransform;
	int id = 0;
	
public:
	Node(int id, std::string name, std::vector<Mesh*> mesh, DirectX::FXMMATRIX & transform);
	void AddNode(std::unique_ptr<Node> node);
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform);
	void DrawAABB(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform);
	void ShowWindow(Node *& selectedNode) const;
	void SetAppliedTransform(DirectX::FXMMATRIX appliedTransform);
    void IntersectNode(const DirectX::XMMATRIX& accumulatedTransform, const tinybvh::Ray& rayWorld, IntersectionResult& closestHit);
	std::string GetName() const;
	int GetId() const;
};

class ModelWindow;
class Model
{
private:
	std::vector<std::unique_ptr<Mesh>> _meshes;
	std::unique_ptr<Node> _root;
	std::string _name = "Sample Text";
	std::unique_ptr<ModelWindow> _pwindow;
	std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh, aiMaterial* const* materials);
	DirectX::XMMATRIX ConvertToMatrix(const aiMatrix4x4& mat);
	std::unique_ptr<Node> ParseNode(int& nextId, const aiNode& node);

public:
	Model(Graphics& gfx, const std::string modelPath);
	void Draw(Graphics& gfx);
	void DrawAABB(Graphics& gfx);
	void ShowWindow();
	IntersectionResult IntersectMesh(const DirectX::XMFLOAT3 rayOriginWorld, const DirectX::XMFLOAT3 rayDirectionWorld);
	~Model();
};