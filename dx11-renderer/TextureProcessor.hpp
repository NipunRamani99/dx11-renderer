#pragma once

#include "Surface.hpp"
#include "ChiliMath.hpp"
#include <string>
#include <DirectXMath.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
class TexturePreprocessor
{

  public:
    template <typename Func> static void TransformSurface( Surface& surf, Func transform )
    {
        for( std::size_t x = 0; x < surf.GetWidth(); x++ )
        {
            for( std::size_t y = 0; y < surf.GetHeight(); y++ )
            {
                const auto n = ColorToVector( surf.GetPixel( (unsigned int)x, (unsigned int)y ) );
                surf.PutPixel( (unsigned int)x, (unsigned int)y, VectorToColor( transform( n, x, y ) ) );
            }
        }
    }

    template <typename Func>
    static void TransformTextureFile( const std::string pIn, const std::string pOut, Func transform )
    {
        using namespace DirectX;
        Surface s = Surface::FromFile( pIn );
        TransformSurface( s, transform );
        s.Save( pOut );
    }

    static void FlipYNormalMap( std::string pIn, std::string pOut )
    {
        using namespace DirectX;
        const auto flipY         = XMVectorSet( 1.0f, -1.0f, 1.0f, 1.0f );
        const auto ProcessNormal = [flipY]( FXMVECTOR n, std::size_t x, std::size_t y ) -> XMVECTOR {
            return XMVectorMultiply( n, flipY );
        };
        TransformTextureFile( pIn, pOut, ProcessNormal );
    }

    static void RotateXAxis180( std::string pIn )
    {
        FlipYNormalMap( pIn, pIn );
    }

    static void FlipYAllNormalMapsInObj( const std::string& pathModel )
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile( pathModel, 0 );
        std::string assetDir = std::filesystem::path{ pathModel }.parent_path().string();
        for( size_t i = 0; i < scene->mNumMaterials; i++ )
        {
            const aiMaterial* mat = scene->mMaterials[i];
            aiString fileName;
            if( mat->GetTexture( aiTextureType::aiTextureType_NORMALS, 0, &fileName ) == aiReturn_SUCCESS )
            {
                RotateXAxis180( assetDir + "/" + fileName.C_Str() );
            }
        }
    }

    void ValidateNormalMap( const std::string& pathIn, float thresholdMin, float thresholdMax )
    {
        OutputDebugStringA( ( "Validating normal map [" + pathIn + "]\n" ).c_str() );
        // function for processing each normal in texture
        using namespace DirectX;
        auto sum                 = XMVectorZero();
        const auto ProcessNormal = [thresholdMin, thresholdMax, &sum]( FXMVECTOR n, std::size_t x,
                                                                       std::size_t y ) -> XMVECTOR {
            const float len = XMVectorGetX( XMVector3Length( n ) );
            const float z   = XMVectorGetZ( n );
            if( len < thresholdMin || len > thresholdMax )
            {
                XMFLOAT3 vec;
                XMStoreFloat3( &vec, n );
                std::ostringstream oss;
                oss << "Bad normal length: " << len << " at: (" << x << "," << y << ") normal: (" << vec.x << ","
                    << vec.y << "," << vec.z << ")\n";
                OutputDebugStringA( oss.str().c_str() );
            }
            if( z < 0.0f )
            {
                XMFLOAT3 vec;
                XMStoreFloat3( &vec, n );
                std::ostringstream oss;
                oss << "Bad normal Z direction at: (" << x << "," << y << ") normal: (" << vec.x << "," << vec.y << ","
                    << vec.z << ")\n";
                OutputDebugStringA( oss.str().c_str() );
            }
            sum = XMVectorAdd( sum, n );
            return n;
        };
        // execute the validation for each texel
        auto surf = Surface::FromFile( pathIn );
        TransformSurface( surf, ProcessNormal );
        // output bias
        {
            XMFLOAT2 sumv;
            XMStoreFloat2( &sumv, sum );
            std::ostringstream oss;
            oss << "Normal map biases: (" << sumv.x << "," << sumv.y << ")\n";
            OutputDebugStringA( oss.str().c_str() );
        }
    }

  private:
    // Map [0, 255] -> (-1.0f, 1.0f)
    // Formula : f(x): x is in [a,b] and f(x) is in [c,d] = c + (d - c)(x - a)/(b - a)
    // Substituting: (2) * (x) / (255) - 1.0f
    static DirectX::FXMVECTOR ColorToVector( const Surface::Color c )
    {
        using namespace DirectX;
        const XMVECTOR all1      = XMVectorReplicate( 1.0f );
        const XMVECTOR all2by255 = XMVectorReplicate( 2.0f / 255.0f );
        const XMVECTOR input     = XMVectorSet( (float)c.GetR(), (float)c.GetG(), (float)c.GetB(), (float)c.GetA() );
        const XMVECTOR output    = XMVectorSubtract( XMVectorMultiply( input, all2by255 ), all1 );
        return output;
    }

    // Map [-1, 1] -> [0, 255]/[0,1]
    // Formula : f(x): x is in [a,b] and f(x) is in [c,d] = c + (d - c)(x - a)/(b - a)
    // Substituting: (255.0f/2.0f) * (x + 1)  / 2
    static Surface::Color VectorToColor( const DirectX::FXMVECTOR v )
    {
        using namespace DirectX;
        const XMVECTOR all1      = XMVectorReplicate( 1.0f );
        const XMVECTOR all255by2 = XMVectorReplicate( 255.0f / 2.0f );
        const XMVECTOR result    = XMVectorMultiply( all255by2, XMVectorAdd( v, all1 ) );
        XMFLOAT3 output;
        XMStoreFloat3( &output, result );
        return { (unsigned char)round( output.x ), (unsigned char)round( output.y ), (unsigned char)round( output.z ) };
    }
};
