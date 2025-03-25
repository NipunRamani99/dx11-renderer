#define TINYBVH_IMPLEMENTATION
#include "Mesh.hpp"
#include "imgui\imgui.h"
#include <unordered_map>
#include "Texture.hpp"
#include "Surface.hpp"
#include "Sampler.hpp"
#include "ChiliDX.hpp"
#include "Blender.hpp"
#include "RasterizerState.hpp"
#include "ConstantBufferEx.hpp"
#include <vector>
#include <stdexcept>

Mesh::Mesh( Graphics& gfx, std::vector<std::shared_ptr<Bind::Bindable>>& bindables, std::unique_ptr<tinybvh::BVH> bvh,
            std::vector<tinybvh::bvhvec4>& vertices, const AABB& aabb, std::string name, std::string shaderName )
    : Mesh( gfx, bindables, aabb, name, shaderName )
{
    this->bvh      = std::move( bvh );
    this->vertices = std::move( vertices );
}

Mesh::Mesh( Graphics& gfx, std::vector<std::shared_ptr<Bind::Bindable>>& bindables, const AABB& aabb, std::string name,
            std::string shaderName )
    : viz( gfx, aabb, name ), _shaderName( shaderName )
{
    using namespace Bind;

    AddBind( std::make_shared<Bind::Topology>( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

    for( auto& pbind : bindables )
    {
        AddBind( pbind );
    }

    AddBind( std::make_unique<TransformCbuf>( gfx, *this ) );
    _aabb = aabb;
}

void Mesh::Update( float ) noexcept {}

void Mesh::Draw( Graphics& gfx, DirectX::XMMATRIX accumulatedTransform ) noexcept
{

    DirectX::XMStoreFloat4x4( &_transform, accumulatedTransform );

    Drawable::Draw( gfx );
}

void Mesh::DrawAABB( Graphics& gfx, DirectX::XMMATRIX accumulatedTransform ) noexcept
{
    viz.SetTransform( accumulatedTransform );
    viz.Draw( gfx );
}

DirectX::XMMATRIX Mesh::GetTransformXM() const noexcept
{
    return DirectX::XMLoadFloat4x4( &_transform );
}

void Mesh::SetAABB( AABB aabb )
{
    _aabb = aabb;
}

const AABB& Mesh::getAABB() const
{
    return _aabb;
}

void Mesh::SetBVH( std::unique_ptr<tinybvh::BVH> pbvh )
{
    this->bvh = std::move( pbvh );
}

const tinybvh::BVH& Mesh::GetBVH()
{
    return *bvh.get();
}

Node::Node( int id, std::string name, std::vector<Mesh*> mesh, DirectX::FXMMATRIX& transform )
    : _mesh( mesh ), _name( name ), id( id )
{
    DirectX::XMStoreFloat4x4( &_basetransform, transform );
    DirectX::XMStoreFloat4x4( &_appliedtransform, DirectX::XMMatrixIdentity() );
}

void Node::AddNode( std::unique_ptr<Node> node )
{
    _nodes.push_back( std::move( node ) );
}

void Node::Draw( Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform = DirectX::XMMatrixIdentity() )
{
    const auto nodeTransform = DirectX::XMLoadFloat4x4( &_appliedtransform ) *
                               DirectX::XMLoadFloat4x4( &_basetransform ) * accumulatedTransform;
    for( auto& pnode : _nodes )
    {
        pnode->Draw( gfx, nodeTransform );
    }
    for( auto pmesh : _mesh )
    {
        pmesh->Draw( gfx, nodeTransform );
    }
}

void Node::DrawAABB( Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform = DirectX::XMMatrixIdentity() )
{
    const auto nodeTransform = DirectX::XMLoadFloat4x4( &_appliedtransform ) *
                               DirectX::XMLoadFloat4x4( &_basetransform ) * accumulatedTransform;
    for( auto pmesh : _mesh )
    {
        pmesh->DrawAABB( gfx, nodeTransform );
    }
    for( auto& pnode : _nodes )
    {
        pnode->DrawAABB( gfx, nodeTransform );
    }
}

void Node::ShowWindow( Graphics& gfx, Node*& selectedNode, std::string windowName ) const
{
    int selectedIndex = -1;
    if( selectedNode != nullptr )
    {
        selectedIndex = selectedNode->GetId();
    }
    ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_OpenOnArrow |
                              ( selectedIndex == GetId() ? ImGuiTreeNodeFlags_Selected : 0 ) |
                              ( _nodes.empty() ? ImGuiTreeNodeFlags_Leaf : 0 );
    if( ImGui::TreeNodeEx( (void*)(intptr_t)GetId(), flag, _name.c_str() ) )
    {
        selectedIndex = ImGui::IsItemClicked() ? GetId() : selectedIndex;
        selectedNode  = ImGui::IsItemClicked() ? const_cast<Node*>( this ) : selectedNode;
        for( auto& pnode : _nodes )
        {
            pnode->ShowWindow( gfx, selectedNode, windowName );
        }
        ImGui::TreePop();
    }
}

void Node::SetAppliedTransform( DirectX::FXMMATRIX appliedTransform )
{
    DirectX::XMStoreFloat4x4( &_appliedtransform, appliedTransform );
}

void Node::IntersectNode( const DirectX::XMMATRIX& accumulatedTransform, const tinybvh::Ray& rayWorld,
                          IntersectionResult& closestHit )
{
    // Compute the node's world transformation matrix
    DirectX::XMMATRIX nodeTransform = DirectX::XMLoadFloat4x4( &_appliedtransform ) *
                                      DirectX::XMLoadFloat4x4( &_basetransform ) * accumulatedTransform;

    // For each Mesh in the Node
    for( const auto& pmesh : _mesh )
    {
        // Get the inverse of the Mesh's world transform
        DirectX::XMMATRIX invTransform = DirectX::XMMatrixInverse( nullptr, nodeTransform );

        // Transform the ray into the Mesh's local space
        DirectX::XMVECTOR rayOriginWorld = DirectX::XMVectorSet( rayWorld.O.x, rayWorld.O.y, rayWorld.O.z, 1.0f );
        DirectX::XMVECTOR rayOriginLocal = DirectX::XMVector3Transform( rayOriginWorld, invTransform );

        DirectX::XMVECTOR rayDirWorld    = DirectX::XMVectorSet( rayWorld.D.x, rayWorld.D.y, rayWorld.D.z, 0.0f );
        DirectX::XMVECTOR rayDirLocal    = DirectX::XMVector3TransformNormal( rayDirWorld, invTransform );
        rayDirLocal                      = DirectX::XMVector3Normalize( rayDirLocal );

        // Convert to tinybvh::Ray
        tinybvh::bvhvec3 rayOriginLocalVec;
        tinybvh::bvhvec3 rayDirLocalVec;
        DirectX::XMStoreFloat3( reinterpret_cast<DirectX::XMFLOAT3*>( &rayOriginLocalVec ), rayOriginLocal );
        DirectX::XMStoreFloat3( reinterpret_cast<DirectX::XMFLOAT3*>( &rayDirLocalVec ), rayDirLocal );

        tinybvh::Ray rayLocal( rayOriginLocalVec, rayDirLocalVec );

        // Perform intersection with the Mesh's BVH
        const tinybvh::BVH& bvh = pmesh->GetBVH();
        bvh.Intersect( rayLocal );

        if( rayLocal.hit.t < closestHit.t && rayLocal.hit.t > 0 )
        {
            // Update closest hit
            closestHit.hit = true;
            closestHit.t   = rayLocal.hit.t;

            // Compute intersection point in local space
            tinybvh::bvhvec3 hitPointLocal = rayLocal.O + rayLocal.D * rayLocal.hit.t;

            // Transform the hit point back to world space
            DirectX::XMVECTOR hitPointLocalVec =
                DirectX::XMVectorSet( hitPointLocal.x, hitPointLocal.y, hitPointLocal.z, 1.0f );
            DirectX::XMVECTOR hitPointWorldVec = DirectX::XMVector3Transform( hitPointLocalVec, nodeTransform );

            DirectX::XMFLOAT3 hitPointWorld;
            DirectX::XMStoreFloat3( &hitPointWorld, hitPointWorldVec );

            closestHit.point         = tinybvh::bvhvec3( hitPointWorld.x, hitPointWorld.y, hitPointWorld.z );
            closestHit.mesh          = pmesh;
            closestHit.node          = this;
            closestHit.triangleIndex = rayLocal.hit.prim;
        }
    }

    // Recurse into child Nodes
    for( const auto& childNode : _nodes )
    {
        childNode->IntersectNode( nodeTransform, rayWorld, closestHit );
    }
}

std::string Node::GetName() const
{
    return _name;
}

int Node::GetId() const
{
    return id;
}

const Dcb::Buffer* Node::GetMaterialConstants() const noxnd
{
    if( _mesh.size() == 0 )
    {
        return nullptr;
    }
    auto pBindable = _mesh.front()->QueryBindable<Bind::CachingPixelConstantBufferEX>();
    return &pBindable->GetBuffer();
}

void Node::SetMaterialConstants( const Dcb::Buffer& buf_in ) noxnd
{
    auto pcb = _mesh.front()->QueryBindable<Bind::CachingPixelConstantBufferEX>();
    assert( pcb != nullptr );
    pcb->SetBuffer( buf_in );
}

class ModelWindow
{
  public:
    ModelWindow() {}
    void ShowWindow( Graphics& gfx, std::string windowName, const Node& rootNode )
    {

        if( ImGui::Begin( windowName.c_str() ) )
        {
            ImGui::Columns( 2, nullptr, true );
            if( ImGui::TreeNodeEx( windowName.c_str() ) )
            {

                rootNode.ShowWindow( gfx, _pselectednode, windowName );
                ImGui::TreePop();
            }
            ImGui::NextColumn();
            selectedIndex = _pselectednode != nullptr ? _pselectednode->GetId() : -1;
            if( selectedIndex != -1 )
            {
                auto i = transforms.find( selectedIndex );
                if( i == transforms.end() )
                {
                    auto transform                = _pselectednode->GetAppliedTransform();
                    DirectX::XMFLOAT3 translation = ChiliDX::ExtractTranslation( transform );
                    DirectX::XMFLOAT3 rotation    = ChiliDX::ExtractPitchYawRoll( transform );
                    DirectX::XMFLOAT3 scale       = ChiliDX::ExtractScale( transform );

                    TransformParams transformParams;
                    transformParams.x     = translation.x;
                    transformParams.y     = translation.y;
                    transformParams.z     = translation.z;
                    transformParams.pitch = rotation.x;
                    transformParams.yaw   = rotation.y;
                    transformParams.roll  = rotation.z;
                    auto pMaterialCBuf    = _pselectednode->GetMaterialConstants();
                    auto buf              = pMaterialCBuf != nullptr ? std::optional<Dcb::Buffer>( *pMaterialCBuf )
                                                                     : std::optional<Dcb::Buffer>();
                    std::tie( i, std::ignore ) =
                        transforms.insert( { selectedIndex, { transformParams, false, std::move( buf ), false } } );
                }
                // link imgui control to cached transform params
                {
                    auto& transforms = i->second.transformParams;

                    // dirty check
                    auto& flag        = i->second.transformParamsDirty;
                    const auto dcheck = [&flag]( bool changed ) { flag = flag || changed; };
                    ImGui::Text( "Selected Index: %d", selectedIndex );
                    ImGui::Text( "Position" );
                    dcheck( ImGui::InputFloat( "Position X", (float*)&transforms.x, 0.1f, 1.0f, "%.3f" ) );
                    dcheck( ImGui::InputFloat( "Position Y", (float*)&transforms.y, 0.1f, 1.0f, "%.3f" ) );
                    dcheck( ImGui::InputFloat( "Position Z", (float*)&transforms.z, 0.1f, 1.0f, "%.3f" ) );
                    ImGui::Text( "Orientation" );
                    dcheck( ImGui::SliderAngle( "Yaw", (float*)&transforms.yaw, -180.0f, 180.0f ) );
                    dcheck( ImGui::SliderAngle( "Pitch", (float*)&transforms.pitch, -180.0f, 180.0f ) );
                    dcheck( ImGui::SliderAngle( "Roll", (float*)&transforms.roll, -180.0f, 180.0f ) );
                }
                // link imgui controls to cached material cbuf params

                {
                    if( i->second.materialCBuf )
                    {
                        auto& mat         = *i->second.materialCBuf;

                        auto& dirty       = i->second.materialCbufDirty;
                        const auto dcheck = [&dirty]( bool changed ) { dirty = dirty || changed; };
                        ImGui::Text( "Material" );
                        if( auto v = mat["normalMapEnabled"]; v.Exists() )
                        {
                            dcheck( ImGui::Checkbox( "Normal Map", &v ) );
                        }
                        if( auto v = mat["specularMapEnabled"]; v.Exists() )
                        {
                            dcheck( ImGui::Checkbox( "Spec Map", &v ) );
                        }
                        if( auto v = mat["hasGlossMap"]; v.Exists() )
                        {
                            dcheck( ImGui::Checkbox( "Gloss Map", &v ) );
                        }
                        if( auto v = mat["materialColor"]; v.Exists() )
                        {
                            dcheck( ImGui::ColorPicker3(
                                "Diff Color", reinterpret_cast<float*>( &static_cast<DirectX::XMFLOAT3&>( v ) ) ) );
                        }
                        if( auto v = mat["specularPower"]; v.Exists() )
                        {
                            dcheck( ImGui::SliderFloat( "Spec Power", &v, 0.0f, 100.0f, "%.1f", 1.5f ) );
                        }
                        if( auto v = mat["specularColor"]; v.Exists() )
                        {
                            dcheck( ImGui::ColorPicker3(
                                "Spec Color", reinterpret_cast<float*>( &static_cast<DirectX::XMFLOAT3&>( v ) ) ) );
                        }
                        if( auto v = mat["specularMapWeight"]; v.Exists() )
                        {
                            dcheck( ImGui::SliderFloat( "Spec Weight", &v, 0.0f, 4.0f ) );
                        }
                        if( auto v = mat["specularIntensity"]; v.Exists() )
                        {
                            dcheck( ImGui::SliderFloat( "Spec Intens", &v, 0.0f, 1.0f ) );
                        }
                    }
                }
            }

            std::string shaderName =
                _pselectednode != nullptr
                    ? _pselectednode->GetMeshes().size() > 0 ? _pselectednode->GetMeshes()[0]->GetShaderName() : ""
                    : "";
            ImGui::Text( "Shader Used: %s", shaderName.c_str() );

            // if( _pselectednode )
            //{
            //     _pselectednode->Control( gfx, normalData );
            //     _pselectednode->Control( gfx, objectData );
            // }
        }
        ImGui::End();
    }

    Node* GetSelectedNode()
    {
        return _pselectednode;
    }

    ~ModelWindow() = default;

    void ApplyParameters() noxnd
	{
		if( TransformDirty() )
		{
			_pselectednode->SetAppliedTransform( GetTransform() );
			ResetTransformDirty();
		}
		if( MaterialDirty() )
		{
			_pselectednode->SetMaterialConstants( GetMaterial() );
			ResetMaterialDirty();
		}
	}

    const Dcb::Buffer& GetMaterial() const noxnd
	{
		assert( _pselectednode != nullptr );
		const auto& mat = transforms.at( _pselectednode->GetId() ).materialCBuf;
		assert( mat );
		return *mat;
	}
    DirectX::XMMATRIX GetTransform() const noxnd
	{
		assert( _pselectednode != nullptr );
		const auto& transform = transforms.at( _pselectednode->GetId() ).transformParams;
		return 
			DirectX::XMMatrixRotationRollPitchYaw( transform.roll,transform.pitch,transform.yaw ) *
			DirectX::XMMatrixTranslation( transform.x,transform.y,transform.z );
	}
    bool TransformDirty() const noxnd
	{
		return _pselectednode && transforms.at( _pselectednode->GetId() ).transformParamsDirty;
	}
	void ResetTransformDirty() noxnd
	{
		transforms.at( _pselectednode->GetId() ).transformParamsDirty = false;
	}
	bool MaterialDirty() const noxnd
	{
		return _pselectednode && transforms.at( _pselectednode->GetId() ).materialCbufDirty;
	}
	void ResetMaterialDirty() noxnd
	{
		transforms.at( _pselectednode->GetId() ).materialCbufDirty = false;
	}
	bool IsDirty() const noxnd
	{
		return TransformDirty() || MaterialDirty();
	}

  private:
    int selectedIndex = -1;
    struct TransformParams
    {
        float yaw   = 0.0f;
        float roll  = 0.0f;
        float pitch = 0.0f;
        float x     = 0.0f;
        float y     = 0.0f;
        float z     = 0.0f;
    };
    struct NodeData
    {
        TransformParams transformParams;
        bool transformParamsDirty;
        std::optional<Dcb::Buffer> materialCBuf;
        bool materialCbufDirty;
    };
    std::unordered_map<int, NodeData> transforms;
    Node* _pselectednode = nullptr;
};

Model::Model( Graphics& gfx, const std::string modelPath, float scale ) : _assetLocation( modelPath )

{
    Assimp::Importer importer;
    const auto pScene = importer.ReadFile( modelPath.c_str(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                                                                  aiProcess_ConvertToLeftHanded | aiProcess_GenNormals |
                                                                  aiProcess_CalcTangentSpace );
    const size_t last_slash_idx =
        _assetLocation.rfind( '\\' ) != std::string::npos ? _assetLocation.rfind( '\\' ) : _assetLocation.rfind( '/' );
    if( std::string::npos != last_slash_idx )
    {
        _assetDir = _assetLocation.substr( 0, last_slash_idx );
        _assetDir += "/";
    }

    _name = "Test";
    _name = _name.empty() ? "Sample Scene" : _name;
    for( size_t i = 0; i < pScene->mNumMeshes; i++ )
    {
        _meshes.push_back( ParseMesh( gfx, *pScene->mMeshes[i], pScene->mMaterials, scale ) );
    }
    int nextId = 0;
    _root      = ParseNode( nextId, *pScene->mRootNode );
    _pwindow   = std::make_unique<ModelWindow>();
}

void Model::Draw( Graphics& gfx )
{
    _pwindow->ApplyParameters();
    _root->Draw( gfx, DirectX::XMMatrixIdentity() );
}

void Model::DrawAABB( Graphics& gfx )
{
    _root->DrawAABB( gfx );
}

std::unique_ptr<Mesh> Model::ParseMesh( Graphics& gfx, const aiMesh& mesh, aiMaterial* const* materials, float scale )
{
    namespace dx = DirectX;
    using namespace Bind;
    std::vector<std::shared_ptr<Bindable>> bindables;
    bool hasDiffuseMap         = false;
    bool hasNormalMap          = false;
    bool hasSpecularMap        = false;
    bool hasGlossMap           = false;
    bool hasAlpha              = false;
    float shininess            = 30.0f;
    dx::XMFLOAT4 specularColor = { 0.18f, 0.18f, 0.18f, 1.0f };
    dx::XMFLOAT3 materialColor = { 1.0f, 0.0f, 1.0f };

    if( mesh.mMaterialIndex >= 0 )
    {
        aiMaterial* material = materials[mesh.mMaterialIndex];
        aiString texFileName;
        if( material->GetTexture( aiTextureType_DIFFUSE, 0, &texFileName ) == aiReturn_SUCCESS )
        {
            auto tex = Texture::Resolve( gfx, _assetDir + "/" + texFileName.C_Str(), 0u );
            bindables.push_back( tex );
            hasDiffuseMap = true;
            hasAlpha      = tex->HasAlpha();
        }
        else
        {
            material->Get( AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>( materialColor ) );
        }
        if( material->GetTexture( aiTextureType_SPECULAR, 0, &texFileName ) == aiReturn_SUCCESS )
        {
            auto tex    = Texture::Resolve( gfx, _assetDir + "/" + texFileName.C_Str(), 1u );
            hasGlossMap = tex->HasAlpha();
            bindables.push_back( tex );
            hasSpecularMap = true;
        }
        else
        {
            material->Get( AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor3D&>( specularColor ) );
        }
        if( !hasGlossMap )
        {
            material->Get( AI_MATKEY_SHININESS, shininess );
        }
        if( material->GetTexture( aiTextureType_NORMALS, 0, &texFileName ) == aiReturn_SUCCESS )
        {
            bindables.push_back( Texture::Resolve( gfx, _assetDir + "/" + texFileName.C_Str(), 2u ) );
            hasNormalMap = true;
        }
        if( hasDiffuseMap || hasSpecularMap || hasNormalMap )
        {
            bindables.push_back( Bind::Sampler::Resolve( gfx, 0u ) );
        }
    }

    std::vector<unsigned int> indices;
    std::vector<tinybvh::bvhvec4> vertices;
    indices.reserve( mesh.mNumFaces * 3 );
    vertices.reserve( mesh.mNumFaces * 3 );
    for( unsigned int i = 0; i < mesh.mNumFaces; i++ )
    {
        const auto& face = mesh.mFaces[i];
        indices.push_back( face.mIndices[0] );
        indices.push_back( face.mIndices[1] );
        indices.push_back( face.mIndices[2] );
        auto v1 = mesh.mVertices[face.mIndices[0]];
        auto v2 = mesh.mVertices[face.mIndices[1]];
        auto v3 = mesh.mVertices[face.mIndices[2]];

        vertices.push_back( { scale * v1.x, scale * v1.y, scale * v1.z, 0 } );
        vertices.push_back( { scale * v2.x, scale * v2.y, scale * v2.z, 0 } );
        vertices.push_back( { scale * v3.x, scale * v3.y, scale * v3.z, 0 } );
    }

    std::unique_ptr<tinybvh::BVH> bvh = std::make_unique<tinybvh::BVH>();
    bvh->Build( vertices.data(), mesh.mNumFaces );
    bindables.push_back( IndexBuffer::Resolve( gfx, mesh.mName.C_Str(), indices ) );
    if( hasAlpha )
        bindables.push_back( RasterizerState::Resolve( gfx, hasAlpha ) );

    Dvtx::VertexLayout layout;
    std::string shaderName = "";
    bindables.push_back( Bind::Blender::Resolve( gfx, false ) );
    if( hasDiffuseMap && hasNormalMap && hasSpecularMap )
    {
        layout.Append<Dvtx::VertexLayout::Position3D>()
            .Append<Dvtx::VertexLayout::Normal>()
            .Append<Dvtx::VertexLayout::Tangent>()
            .Append<Dvtx::VertexLayout::BiTangent>()
            .Append<Dvtx::VertexLayout::Texture2D>();
        Dvtx::VertexBuffer vbuf( std::move( layout ) );
        bindables.push_back(
            PixelShader::Resolve( gfx, hasAlpha ? "PhongPSSpecNormMask.cso" : "PhongPSSpecNormMap.cso" ) );

        shaderName = hasAlpha ? "PhongPSSpecNormMask.cso" : "PhongPSSpecNormMap.cso";

        AABB aabb;
        aabb.min.x = scale * mesh.mVertices[0].x;
        aabb.min.y = scale * mesh.mVertices[0].y;
        aabb.min.z = scale * mesh.mVertices[0].z;
        aabb.max.x = scale * mesh.mVertices[0].x;
        aabb.max.y = scale * mesh.mVertices[0].y;
        aabb.max.z = scale * mesh.mVertices[0].z;

        for( unsigned int i = 0; i < mesh.mNumVertices; i++ )
        {
            aabb.min.x = std::min( scale * mesh.mVertices[i].x, scale * aabb.min.x );
            aabb.min.y = std::min( scale * mesh.mVertices[i].y, scale * aabb.min.y );
            aabb.min.z = std::min( scale * mesh.mVertices[i].z, scale * aabb.min.z );
            aabb.max.x = std::max( scale * mesh.mVertices[i].x, scale * aabb.max.x );
            aabb.max.y = std::max( scale * mesh.mVertices[i].y, scale * aabb.max.y );
            aabb.max.z = std::max( scale * mesh.mVertices[i].z, scale * aabb.max.z );
            vbuf.EmplaceBack(
                dx::XMFLOAT3( mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale ),
                *reinterpret_cast<dx::XMFLOAT3*>( &mesh.mNormals[i] ),
                *reinterpret_cast<dx::XMFLOAT3*>( &mesh.mTangents[i] ),
                *reinterpret_cast<dx::XMFLOAT3*>( &mesh.mBitangents[i] ),
                *reinterpret_cast<dx::XMFLOAT2*>( &mesh.mTextureCoords[0][i] ) );
        }

        bindables.push_back( VertexBuffer::Resolve( gfx, mesh.mName.C_Str(), vbuf ) );

        auto vs   = VertexShader::Resolve( gfx, "PhongVSTexturedTBN.cso" );
        auto vsbc = vs->GetBytecode();
        bindables.push_back( vs );
        bindables.push_back( InputLayout::Resolve( gfx, vbuf.GetVertexLayout(), vsbc ) );

        Dcb::RawLayout constBufLayout;
        constBufLayout.Add<Dcb::Float>( "specularIntensity" );
        constBufLayout.Add<Dcb::Float>( "specularPower" );
        constBufLayout.Add<Dcb::Bool>( "normalMapEnabled" );
        constBufLayout.Add<Dcb::Bool>( "specularMapEnabled" );
        constBufLayout.Add<Dcb::Bool>( "hasGloss" );
        constBufLayout.Add<Dcb::Float3>( "specularColor" );
        constBufLayout.Add<Dcb::Float>( "specularMapWeight" );

        auto buf                 = Dcb::Buffer( std::move( constBufLayout ) );
        buf["specularIntensity"] = 0.60f;
        buf["specularPower"]     = 1.0f;
        buf["normalMapEnabled"]      = hasNormalMap;
        buf["specularMapEnabled"]    = hasSpecularMap;
        buf["hasGloss"]          = hasGlossMap;
        buf["specularColor"]     = DirectX::XMFLOAT3( specularColor.x, specularColor.y, specularColor.z );
        buf["specularMapWeight"] = 0.671f;
        bindables.push_back( std::make_shared<Bind::CachingPixelConstantBufferEX>( gfx, buf, 4u ) );

        return make_unique<Mesh>( gfx, bindables, std::move( bvh ), vertices, aabb, mesh.mName.C_Str(), shaderName );
    }
    else if( hasDiffuseMap && hasNormalMap )
    {
        layout.Append<Dvtx::VertexLayout::Position3D>()
            .Append<Dvtx::VertexLayout::Normal>()
            .Append<Dvtx::VertexLayout::Tangent>()
            .Append<Dvtx::VertexLayout::BiTangent>()
            .Append<Dvtx::VertexLayout::Texture2D>();
        Dvtx::VertexBuffer vbuf( std::move( layout ) );

        bindables.push_back( PixelShader::Resolve( gfx, "PhongPSNormalMap.cso" ) );
        shaderName = "PhongPSNormalMap";
        AABB aabb;
        aabb.min.x = scale * mesh.mVertices[0].x;
        aabb.min.y = scale * mesh.mVertices[0].y;
        aabb.min.z = scale * mesh.mVertices[0].z;
        aabb.max.x = scale * mesh.mVertices[0].x;
        aabb.max.y = scale * mesh.mVertices[0].y;
        aabb.max.z = scale * mesh.mVertices[0].z;

        for( unsigned int i = 0; i < mesh.mNumVertices; i++ )
        {
            aabb.min.x = std::min( scale * mesh.mVertices[i].x, scale * aabb.min.x );
            aabb.min.y = std::min( scale * mesh.mVertices[i].y, scale * aabb.min.y );
            aabb.min.z = std::min( scale * mesh.mVertices[i].z, scale * aabb.min.z );
            aabb.max.x = std::max( scale * mesh.mVertices[i].x, scale * aabb.max.x );
            aabb.max.y = std::max( scale * mesh.mVertices[i].y, scale * aabb.max.y );
            aabb.max.z = std::max( scale * mesh.mVertices[i].z, scale * aabb.max.z );
            vbuf.EmplaceBack(
                dx::XMFLOAT3( mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale ),
                *reinterpret_cast<dx::XMFLOAT3*>( &mesh.mNormals[i] ),
                *reinterpret_cast<dx::XMFLOAT3*>( &mesh.mTangents[i] ),
                *reinterpret_cast<dx::XMFLOAT3*>( &mesh.mBitangents[i] ),
                *reinterpret_cast<dx::XMFLOAT2*>( &mesh.mTextureCoords[0][i] ) );
        }

        bindables.push_back( VertexBuffer::Resolve( gfx, mesh.mName.C_Str(), vbuf ) );

        auto vs   = VertexShader::Resolve( gfx, "PhongVSTexturedTBN.cso" );
        auto vsbc = vs->GetBytecode();
        bindables.push_back( vs );
        bindables.push_back( InputLayout::Resolve( gfx, vbuf.GetVertexLayout(), vsbc ) );

        Dcb::RawLayout constBufLayout;
        constBufLayout.Add<Dcb::Float>( "specularIntensity" );
        constBufLayout.Add<Dcb::Float>( "specularPower" );
        constBufLayout.Add<Dcb::Bool>( "normalMapEnabled" );
        auto buf                 = Dcb::Buffer( std::move( constBufLayout ) );
        buf["specularIntensity"] = ( specularColor.x + specularColor.y + specularColor.z ) / 3.0f;
        buf["specularPower"]     = 1.0f;
        buf["normalMapEnabled"]  = hasNormalMap;

        bindables.push_back( std::make_shared<Bind::CachingPixelConstantBufferEX>( gfx, buf, 4u ) );
        return make_unique<Mesh>( gfx, bindables, std::move( bvh ), vertices, aabb, mesh.mName.C_Str(), shaderName );
    }
    else if( hasDiffuseMap && !hasNormalMap && hasSpecularMap )
    {
        layout.Append<Dvtx::VertexLayout::Position3D>()
            .Append<Dvtx::VertexLayout::Normal>()
            .Append<Dvtx::VertexLayout::Texture2D>();
        Dvtx::VertexBuffer vbuf( std::move( layout ) );

        bindables.push_back( PixelShader::Resolve( gfx, "PhongPSSpecMap.cso" ) );
        shaderName = "PhongPSSpecMap";
        AABB aabb;
        aabb.min.x = scale * mesh.mVertices[0].x;
        aabb.min.y = scale * mesh.mVertices[0].y;
        aabb.min.z = scale * mesh.mVertices[0].z;
        aabb.max.x = scale * mesh.mVertices[0].x;
        aabb.max.y = scale * mesh.mVertices[0].y;
        aabb.max.z = scale * mesh.mVertices[0].z;

        for( unsigned int i = 0; i < mesh.mNumVertices; i++ )
        {
            aabb.min.x = std::min( scale * mesh.mVertices[i].x, scale * aabb.min.x );
            aabb.min.y = std::min( scale * mesh.mVertices[i].y, scale * aabb.min.y );
            aabb.min.z = std::min( scale * mesh.mVertices[i].z, scale * aabb.min.z );
            aabb.max.x = std::max( scale * mesh.mVertices[i].x, scale * aabb.max.x );
            aabb.max.y = std::max( scale * mesh.mVertices[i].y, scale * aabb.max.y );
            aabb.max.z = std::max( scale * mesh.mVertices[i].z, scale * aabb.max.z );
            vbuf.EmplaceBack(
                dx::XMFLOAT3( mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale ),
                *reinterpret_cast<dx::XMFLOAT3*>( &mesh.mNormals[i] ),
                *reinterpret_cast<dx::XMFLOAT2*>( &mesh.mTextureCoords[0][i] ) );
        }

        bindables.push_back( VertexBuffer::Resolve( gfx, mesh.mName.C_Str(), vbuf ) );

        auto vs   = VertexShader::Resolve( gfx, "PhongVSTextured.cso" );
        auto vsbc = vs->GetBytecode();
        bindables.push_back( vs );
        bindables.push_back( InputLayout::Resolve( gfx, vbuf.GetVertexLayout(), vsbc ) );

        Dcb::RawLayout constBufLayout;
        constBufLayout.Add<Dcb::Float3>( "material" );
        constBufLayout.Add<Dcb::Float>( "specularIntensity" );
        constBufLayout.Add<Dcb::Float>( "specularPower" );
        auto buf                 = Dcb::Buffer( std::move( constBufLayout ) );
        buf["material"]          = DirectX::XMFLOAT3{ 1.0f, 0.2f, 0.1f };
        buf["specularIntensity"] = 0.60f;
        buf["specularPower"]     = 120.0f;

        bindables.push_back( std::make_shared<Bind::CachingPixelConstantBufferEX>( gfx, buf, 1u ) );
        return make_unique<Mesh>( gfx, bindables, std::move( bvh ), vertices, aabb, mesh.mName.C_Str(), shaderName );
    }
    else if( hasDiffuseMap )
    {
        layout.Append<Dvtx::VertexLayout::Position3D>()
            .Append<Dvtx::VertexLayout::Normal>()
            .Append<Dvtx::VertexLayout::Texture2D>();
        Dvtx::VertexBuffer vbuf( std::move( layout ) );

        bindables.push_back( PixelShader::Resolve( gfx, "PhongPSTextured.cso" ) );
        shaderName = "PhongPSTextured";
        AABB aabb;
        aabb.min.x = scale * mesh.mVertices[0].x;
        aabb.min.y = scale * mesh.mVertices[0].y;
        aabb.min.z = scale * mesh.mVertices[0].z;
        aabb.max.x = scale * mesh.mVertices[0].x;
        aabb.max.y = scale * mesh.mVertices[0].y;
        aabb.max.z = scale * mesh.mVertices[0].z;

        for( unsigned int i = 0; i < mesh.mNumVertices; i++ )
        {
            aabb.min.x = std::min( scale * mesh.mVertices[i].x, scale * aabb.min.x );
            aabb.min.y = std::min( scale * mesh.mVertices[i].y, scale * aabb.min.y );
            aabb.min.z = std::min( scale * mesh.mVertices[i].z, scale * aabb.min.z );
            aabb.max.x = std::max( scale * mesh.mVertices[i].x, scale * aabb.max.x );
            aabb.max.y = std::max( scale * mesh.mVertices[i].y, scale * aabb.max.y );
            aabb.max.z = std::max( scale * mesh.mVertices[i].z, scale * aabb.max.z );
            vbuf.EmplaceBack(
                dx::XMFLOAT3( mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale ),
                *reinterpret_cast<dx::XMFLOAT3*>( &mesh.mNormals[i] ),
                *reinterpret_cast<dx::XMFLOAT2*>( &mesh.mTextureCoords[0][i] ) );
        }

        bindables.push_back( VertexBuffer::Resolve( gfx, mesh.mName.C_Str(), vbuf ) );

        auto vs   = VertexShader::Resolve( gfx, "PhongVSTextured.cso" );
        auto vsbc = vs->GetBytecode();
        bindables.push_back( vs );
        bindables.push_back( InputLayout::Resolve( gfx, vbuf.GetVertexLayout(), vsbc ) );
        Dcb::RawLayout constBufLayout;
        constBufLayout.Add<Dcb::Float>( "specularIntensity" );
        constBufLayout.Add<Dcb::Float>( "specularPower" );
        auto buf                 = Dcb::Buffer( std::move( constBufLayout ) );
        buf["specularPower"]     = 120.0f;
        buf["specularIntensity"] = 0.60f;

        bindables.push_back( std::make_shared<Bind::CachingPixelConstantBufferEX>( gfx, buf, 1u ) );
        return make_unique<Mesh>( gfx, bindables, std::move( bvh ), vertices, aabb, mesh.mName.C_Str(), shaderName );
    }
    else if( !hasDiffuseMap && !hasSpecularMap && !hasNormalMap )
    {
        layout.Append<Dvtx::VertexLayout::Position3D>().Append<Dvtx::VertexLayout::Normal>();
        Dvtx::VertexBuffer vbuf( std::move( layout ) );

        bindables.push_back( PixelShader::Resolve( gfx, "PhongPS.cso" ) );
        shaderName = "PhongPS";
        AABB aabb;
        aabb.min.x = scale * mesh.mVertices[0].x;
        aabb.min.y = scale * mesh.mVertices[0].y;
        aabb.min.z = scale * mesh.mVertices[0].z;
        aabb.max.x = scale * mesh.mVertices[0].x;
        aabb.max.y = scale * mesh.mVertices[0].y;
        aabb.max.z = scale * mesh.mVertices[0].z;

        for( unsigned int i = 0; i < mesh.mNumVertices; i++ )
        {
            aabb.min.x = std::min( scale * mesh.mVertices[i].x, scale * aabb.min.x );
            aabb.min.y = std::min( scale * mesh.mVertices[i].y, scale * aabb.min.y );
            aabb.min.z = std::min( scale * mesh.mVertices[i].z, scale * aabb.min.z );
            aabb.max.x = std::max( scale * mesh.mVertices[i].x, scale * aabb.max.x );
            aabb.max.y = std::max( scale * mesh.mVertices[i].y, scale * aabb.max.y );
            aabb.max.z = std::max( scale * mesh.mVertices[i].z, scale * aabb.max.z );
            vbuf.EmplaceBack(
                dx::XMFLOAT3( mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale ),
                *reinterpret_cast<dx::XMFLOAT3*>( &mesh.mNormals[i] ) );
        }
        bindables.push_back( VertexBuffer::Resolve( gfx, mesh.mName.C_Str(), vbuf ) );

        Dcb::RawLayout objectDataLayout;
        objectDataLayout.Add<Dcb::Float3>( "material" );
        objectDataLayout.Add<Dcb::Float>( "specularIntensity" );
        objectDataLayout.Add<Dcb::Float>( "specularPower" );
        auto buf                 = Dcb::Buffer( std::move( objectDataLayout ) );
        buf["material"]          = DirectX::XMFLOAT3{ 1.0f, 0.2f, 0.1f };
        buf["specularPower"]     = shininess;
        buf["specularIntensity"] = 0.60f;
        bindables.push_back( std::make_shared<Bind::CachingPixelConstantBufferEX>( gfx, buf, 1u ) );

        auto vs   = VertexShader::Resolve( gfx, "PhongVS.cso" );
        auto vsbc = vs->GetBytecode();
        bindables.push_back( vs );
        bindables.push_back( InputLayout::Resolve( gfx, vbuf.GetVertexLayout(), vsbc ) );
        return make_unique<Mesh>( gfx, bindables, std::move( bvh ), vertices, aabb, mesh.mName.C_Str(), shaderName );
    }
    else
    {
        throw std::runtime_error( "terrible combination of textures in material smh" );
    }
}

DirectX::XMMATRIX Model::ConvertToMatrix( const aiMatrix4x4& mat )
{
    return DirectX::XMMATRIX( mat.a1, mat.a2, mat.a3, mat.a4, mat.b1, mat.b2, mat.b3, mat.b4, mat.c1, mat.c2, mat.c3,
                              mat.c4, mat.d1, mat.d2, mat.d3, mat.d4 );
}

std::unique_ptr<Node> Model::ParseNode( int& nextId, const aiNode& node )
{
    std::vector<Mesh*> meshes;

    for( size_t i = 0; i < node.mNumMeshes; i++ )
    {
        meshes.push_back( _meshes[node.mMeshes[i]].get() );
    }
    std::string name         = node.mName.C_Str();
    name                     = name.empty() ? "Placeholder" : name;
    DirectX::XMMATRIX matrix = DirectX::XMMatrixTranspose(
        DirectX::XMLoadFloat4x4( reinterpret_cast<const DirectX::XMFLOAT4X4*>( &node.mTransformation ) ) );

    std::unique_ptr<Node> pNode = std::make_unique<Node>( nextId++, name, meshes, matrix );

    for( size_t i = 0; i < node.mNumChildren; i++ )
    {
        std::unique_ptr<Node> ch = ParseNode( nextId, *node.mChildren[i] );
        pNode->AddNode( std::move( ch ) );
    }
    return pNode;
}

void Model::ShowWindow( Graphics& gfx, std::string windowName )
{
    _pwindow->ShowWindow( gfx, windowName, *_root );
}

IntersectionResult Model::IntersectMesh( const DirectX::XMFLOAT3 rayOriginWorld,
                                         const DirectX::XMFLOAT3 rayDirectionWorld )
{
    tinybvh::bvhvec3& rayOrigin =
        *reinterpret_cast<tinybvh::bvhvec3*>( const_cast<DirectX::XMFLOAT3*>( &rayOriginWorld ) );
    tinybvh::bvhvec3& rayDirection =
        *reinterpret_cast<tinybvh::bvhvec3*>( const_cast<DirectX::XMFLOAT3*>( &rayDirectionWorld ) );
    tinybvh::Ray ray{ rayOrigin, rayDirection };
    IntersectionResult result;
    if( _root )
    {
        _root->IntersectNode( DirectX::XMMatrixIdentity(), ray, result );
    }
    return result;
}

Model::~Model() = default;
