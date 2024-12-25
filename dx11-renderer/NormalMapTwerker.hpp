#pragma once

#include "Surface.hpp"
#include "ChiliMath.hpp"
#include <string>
#include <DirectXMath.h>

class NormalMapTwerker
{

public:
	static void RotateXAxis180(std::string pIn, std::string pOut)
	{
		using namespace DirectX;
		Surface s = Surface::FromFile(pIn);
		const auto rotation = XMMatrixRotationX(PI);
		auto pBegin = s.GetBufferPtr();
		auto pixels = s.GetWidth() * s.GetHeight();
		auto pEnd = s.GetBufferPtrConst() + pixels;

		for (auto pCurrent = pBegin ; pCurrent < pEnd; pCurrent++)
		{
			Surface::Color c = *pCurrent;
			XMVECTOR v = ColorToVector(c);
			v = XMVector3Transform(v, rotation);
			*pCurrent = VectorToColor(v);
		}

		s.Save(pOut);
	}

	static void RotateXAxis180(std::string pIn)
	{
		RotateXAxis180(pIn, pIn);
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

	// Map [0, 255] -> (-1.0f, 1.0f)
	// Formula : f(x): x is in [a,b] and f(x) is in [c,d] = c + (d - c)(x - a)/(b - a)
	// Substituting: (255.0f/2.0f) * (x + 1)
	static Surface::Color VectorToColor(const DirectX::FXMVECTOR v)
	{
		using namespace DirectX;
		const XMVECTOR all1 = XMVectorReplicate(1.0f);
		const XMVECTOR all255by2 = XMVectorReplicate(255.0f/2.0f);
		const XMVECTOR result = XMVectorMultiply(all255by2, XMVectorAdd(v, all1));
		XMFLOAT3 output;
		XMStoreFloat3(&output, result);
		return {(char)round(output.x), (char) round(output.y), (char) round(output.z)};
	}
};
