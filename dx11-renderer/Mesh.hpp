#pragma once
#include "BindableBase.hpp"
#include "Vertex.h"
#include "AABB.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "AABBVisualisation.hpp"
#pragma warning( push, 0 )
#include "tiny_bvh\tiny_bvh.h"
#pragma warning( pop )
#include <memory>
#include <optional>
#include "imgui\imgui.h"

class Mesh : public Drawable
{
  private:
    mutable DirectX::XMFLOAT4X4 _transform;
    AABB _aabb;
    AABBVisualisation viz;
    std::unique_ptr<tinybvh::BVH> bvh;
    std::vector<tinybvh::bvhvec4> vertices;
    std::string _shaderName = "";

  public:
    Mesh( Graphics& gfx, std::vector<std::shared_ptr<Bind::Bindable>>& bindables, std::unique_ptr<tinybvh::BVH> bvh,
          std::vector<tinybvh::bvhvec4>& vertices, const AABB& aabb = AABB(), std::string name = "",
          std::string shaderName = "" );
    Mesh( Graphics& gfx, std::vector<std::shared_ptr<Bind::Bindable>>& bindables, const AABB& aabb = AABB(),
          std::string name = "", std::string shaderName = "" );
    void Update( float ) noexcept;
    void Draw( Graphics& gfx, DirectX::XMMATRIX accumulatedTransform ) noexcept;
    void DrawAABB( Graphics& gfx, DirectX::XMMATRIX accumulatedTransform ) noexcept;
    DirectX::XMMATRIX GetTransformXM() const noexcept override;
    void SetAABB( AABB aabb );
    const AABB& getAABB() const;
    void SetBVH( std::unique_ptr<tinybvh::BVH> bvh );
    const tinybvh::BVH& GetBVH();
    const std::string GetShaderName() const noexcept
    {
        return _shaderName;
    }
};

class Node;

// Define a struct to hold intersection details
struct IntersectionResult
{
    bool hit = false;
    float t  = 1e30f;                     // Initialize to max float value
    tinybvh::bvhvec3 point;               // Intersection point in world space
    Mesh* mesh                 = nullptr; // Pointer to the Mesh that was hit
    Node* node                 = nullptr; // Pointer to the Node that was hit
    unsigned int triangleIndex = 0;       // Index of the triangle hit
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
    struct ObjectData
    {
        alignas( 16 ) DirectX::XMFLOAT3 material = { 0.447970f, 0.327254f, 0.176283f };
        float specularIntensity                  = 0.60f;
        float specularPower                      = 120.0f;
    };

    struct NormalData
    {
        alignas( 16 ) BOOL hasNormalMap = TRUE;
        BOOL hasSpecularMap             = TRUE;
        BOOL negateXAndY                = FALSE;
        BOOL hasGloss                   = FALSE;
        DirectX::XMFLOAT3 specularColor = { 0.75f, 0.75f, 0.75f };
        float specularMapWeight         = 1.0f;
    };

  public:
    Node( int id, std::string name, std::vector<Mesh*> mesh, DirectX::FXMMATRIX& transform );
    void AddNode( std::unique_ptr<Node> node );
    void Draw( Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform );
    void DrawAABB( Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform );
    void ShowWindow( Graphics& gfx, Node*& selectedNode, std::string windowName = "model" ) const;
    void SetAppliedTransform( DirectX::FXMMATRIX appliedTransform );
    void IntersectNode( const DirectX::XMMATRIX& accumulatedTransform, const tinybvh::Ray& rayWorld,
                        IntersectionResult& closestHit );
    std::string GetName() const;
    int GetId() const;
    const std::vector<Mesh*> GetMeshes() const noexcept
    {
        return _mesh;
    }
    DirectX::XMFLOAT4X4 GetAppliedTransform() const noexcept
    {
        return _appliedtransform;
    }

    template <typename T> bool Control( Graphics& gfx, T& c )
    {
        bool retn = false;
        if( _mesh.empty() )
        {
            return false;
        }
        if constexpr( std::is_same<T, NormalData>::value )
        {
            if( Bind::PixelConstantBuffer<T>* pcb = _mesh.front()->QueryBindable<Bind::PixelConstantBuffer<T>>() )
            {
                ImGui::Text( "Material" );
                bool hasNormalMap = (bool)c.hasNormalMap;
                ImGui::Checkbox( "Norm Map", &hasNormalMap );
                c.hasNormalMap   = hasNormalMap ? TRUE : FALSE;

                bool negateXAndY = (bool)c.negateXAndY;
                ImGui::Checkbox( "Negate Normal Map X and Y: ", &negateXAndY );
                c.negateXAndY = negateXAndY ? TRUE : FALSE;
                pcb->Update( gfx, c );
                retn = true;
            }
        }
        if constexpr( std::is_same<T, ObjectData>::value )
        {
            if( Bind::PixelConstantBuffer<T>* pcb = _mesh.front()->QueryBindable<Bind::PixelConstantBuffer<T>>() )
            {
                ImGui::Text( "Material" );

                ImGui::ColorPicker3( "Material: ", reinterpret_cast<float*>( &c.material ) );

                pcb->Update( gfx, c );
                retn = true;
            }
        }
        return retn;
    }
};

class ModelWindow;
class Model
{
  private:
    std::vector<std::unique_ptr<Mesh>> _meshes;
    std::unique_ptr<Node> _root;
    std::string _name = "Sample Text";
    std::unique_ptr<ModelWindow> _pwindow;
    std::unique_ptr<Mesh> ParseMesh( Graphics& gfx, const aiMesh& mesh, aiMaterial* const* materials, float scale );
    DirectX::XMMATRIX ConvertToMatrix( const aiMatrix4x4& mat );
    std::unique_ptr<Node> ParseNode( int& nextId, const aiNode& node );
    std::string _assetLocation = "";
    std::string _assetDir      = "";

  public:
    Model( Graphics& gfx, const std::string modelPath, float scale = 1.0f );
    void Draw( Graphics& gfx );
    void DrawAABB( Graphics& gfx );
    void ShowWindow( Graphics& gfx, std::string windowName = "model" );
    void Transform( DirectX::FXMMATRIX& transform )
    {
        auto nodeTransform = DirectX::XMLoadFloat4x4( &_root->_appliedtransform );
        nodeTransform      = DirectX::XMMatrixMultiply( transform, nodeTransform );
        DirectX::XMStoreFloat4x4( &_root->_appliedtransform, nodeTransform );
    }
    IntersectionResult IntersectMesh( const DirectX::XMFLOAT3 rayOriginWorld,
                                      const DirectX::XMFLOAT3 rayDirectionWorld );
    ~Model();
};
