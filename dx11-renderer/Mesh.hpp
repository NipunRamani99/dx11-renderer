#pragma once
#include "DrawableBase.hpp"
#include "BindableBase.hpp"
#include "Vertex.h"
#include "AABB.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "AABBVisualisation.hpp"
#include <memory>
#include <optional>
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
	std::string name;
	std::vector<std::unique_ptr<Node>> _nodes;
	DirectX::XMFLOAT4X4 _basetransform;
	DirectX::XMFLOAT4X4 _appliedtransform;

public:
	
	Node(std::string name, std::vector<Mesh*> mesh, DirectX::FXMMATRIX & transform);

	void AddNode(std::unique_ptr<Node> node);

	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform);

	void DrawAABB(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform);

	void ShowWindow(int & nodeIndex, std::optional<int> & selectedIndex, Node *& selectedNode) const;

	void SetAppliedTransform(DirectX::FXMMATRIX appliedTransform);
};
class ModelWindow;
class Model
{
private:
	std::vector<std::unique_ptr<Mesh>> _meshes;
	std::unique_ptr<Node> _root;
	std::string _name = "Sample Text";
	std::unique_ptr<ModelWindow> _pwindow;
public:
	Model(Graphics& gfx, const std::string modelPath);

	void Draw(Graphics& gfx);

	void DrawAABB(Graphics& gfx);

	void ShowWindow();
	~Model();
private:
	std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh);

	DirectX::XMMATRIX ConvertToMatrix(const aiMatrix4x4& mat);

	std::unique_ptr<Node> ParseNode(const aiNode& node);
};