#pragma once

#include "Surface.hpp"
#include "ChiliMath.hpp"
#include <string>
#include <DirectXMath.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
class TextureProcessor
{

public:
	template<typename Func>
	static void TransformTexture(const std::string pIn, const std::string pOut, Func transform)
	{
		using namespace DirectX;
		Surface s = Surface::FromFile(pIn);
		auto pBegin = s.GetBufferPtr();
		auto pixels = s.GetWidth() * s.GetHeight();
		auto pEnd = s.GetBufferPtrConst() + pixels;

		for (auto pCurrent = pBegin; pCurrent < pEnd; pCurrent++)
		{
			const auto n = ColorToVector(*pCurrent);
			*pCurrent = VectorToColor(transform(n));
		}

		s.Save(pOut);
	}

	static void RotateXAxis180(std::string pIn, std::string pOut)
	{
		using namespace DirectX;
		const auto flipY = XMVectorSet(1.0f, -1.0f, 1.0f, 1.0f);
		const auto ProcessNormal = [flipY](FXMVECTOR n) -> XMVECTOR
			{
				return XMVectorMultiply(n, flipY);
			};
		TransformTexture(pIn, pOut, ProcessNormal);
	}

	static void RotateXAxis180(std::string pIn)
	{
		RotateXAxis180(pIn, pIn);
	}

	static void ProcessModel(const std::string pathModel)
	{
		Assimp::Importer importer;
		const aiScene * scene = importer.ReadFile(pathModel, 0);
		std::string assetDir = std::filesystem::path{ pathModel }.parent_path().string();
		for (size_t i = 0; i < scene->mNumMaterials; i++)
		{
			const aiMaterial* mat = scene->mMaterials[i];
			aiString fileName;
			if (mat->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &fileName) == aiReturn_SUCCESS)
			{
				RotateXAxis180(assetDir + "/" + fileName.C_Str());
			}			
		}
	}
private:

	// Map [0, 255] -> (-1.0f, 1.0f)
	// Formula : f(x): x is in [a,b] and f(x) is in [c,d] = c + (d - c)(x - a)/(b - a)
	// Substituting: (2) * (x) / (255) - 1.0f
	static DirectX::FXMVECTOR ColorToVector(const Surface::Color c)
	{
		using namespace DirectX;
		const XMVECTOR all1 = XMVectorReplicate(1.0f);
		const XMVECTOR all2by255 = XMVectorReplicate(2.0f / 255.0f);
		const XMVECTOR input = XMVectorSet((float)c.GetR(), (float)c.GetG(), (float)c.GetB(), (float)c.GetA());
		const XMVECTOR output = XMVectorSubtract(XMVectorMultiply(input, all2by255), all1);
		return output;
	}

	// Map [-1, 1] -> [0, 255]/[0,1]
	// Formula : f(x): x is in [a,b] and f(x) is in [c,d] = c + (d - c)(x - a)/(b - a)
	// Substituting: (255.0f/2.0f) * (x + 1)  / 2 
	static Surface::Color VectorToColor(const DirectX::FXMVECTOR v)
	{
		using namespace DirectX;
		const XMVECTOR all1 = XMVectorReplicate(1.0f);
		const XMVECTOR all255by2 = XMVectorReplicate(255.0f/2.0f);
		const XMVECTOR result = XMVectorMultiply(all255by2, XMVectorAdd(v, all1));
		XMFLOAT3 output;
		XMStoreFloat3(&output, result);
		return {(unsigned char)round(output.x), (unsigned char) round(output.y), (unsigned char) round(output.z)};
	}
};
