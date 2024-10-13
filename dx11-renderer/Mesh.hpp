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
	Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bind::Bindable>>& bindables, const AABB & aabb = AABB());

	void Update(float) noexcept;

	void Draw(Graphics & gfx, DirectX::XMMATRIX accumulatedTransform) noexcept;

	void DrawAABB(Graphics& gfx, DirectX::XMMATRIX accumulatedTransform) noexcept;

	DirectX::XMMATRIX GetTransformXM() const noexcept override;

	void SetAABB(AABB aabb);

	const AABB& getAABB() const;
};

class Node
{
	friend class Model;
private:
	std::vector<Mesh*> _mesh;
	std::vector<std::unique_ptr<Node>> _nodes;
	DirectX::XMFLOAT4X4 _transform;

public:
	
	Node(std::vector<Mesh*> mesh, DirectX::FXMMATRIX & transform);

	void AddNode(std::unique_ptr<Node> node);

	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform);

	void DrawAABB(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform);
};

class Model
{
private:
	std::vector<std::unique_ptr<Mesh>> _meshes;
	std::unique_ptr<Node> _root;
	
public:
	Model(Graphics& gfx, const std::string modelPath);

	void Draw(Graphics& gfx);

	void DrawAABB(Graphics& gfx);

private:
	std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh);

	DirectX::XMMATRIX ConvertToMatrix(const aiMatrix4x4& mat);

	std::unique_ptr<Node> ParseNode(const aiNode& node);
};