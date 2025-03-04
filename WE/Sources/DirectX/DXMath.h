//***************************************************************************************
// MathHelper.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Helper math class.
//***************************************************************************************

#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <cstdint>

using namespace DirectX;

class FTransform
{
public:
	const static FTransform Zeros;
	const static FTransform Default;

public:
	FTransform() :
		Scale(XMFLOAT3(1.0f, 1.0f, 1.0f)),
		Rotation(XMFLOAT3(0.0f, 0.0f, 0.0f)),
		Translation(XMFLOAT3(0.0f, 0.0f, 0.0f)) 
	{}
	FTransform(XMFLOAT3 Scale, XMFLOAT3 Rotation, XMFLOAT3 Translation);
	~FTransform() = default;
	XMMATRIX GetTransformMatrix();

	XMFLOAT3 Scale;
	XMFLOAT3 Rotation;
	XMFLOAT3 Translation;
public:
	inline XMVECTOR GetScaleXMVECTOR() 
	{ 
		return XMLoadFloat3(&Scale); 
	}
	inline XMVECTOR GetRotationXMVECTOR() 
	{ 
		return XMLoadFloat3(&Rotation); 
	}
	inline XMVECTOR GetTranslationXMVECTOR() 
	{ 
		return XMLoadFloat3(&Translation); 
	}
};

class FDXMath
{
public:
	// Returns the polar angle of the point (x,y) in [0, 2*PI).
	static float AngleFromXY(float x, float y);
	static DirectX::XMVECTOR RandUnitVec3();
	static DirectX::XMVECTOR RandHemisphereUnitVec3(DirectX::XMVECTOR n);

	static const float Infinity;
	static const float Pi;

public:
	// Returns random float in [0, 1).
	inline static float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// Returns random float in [a, b).
	inline static float RandF(float a, float b)
	{
		return a + RandF()*(b-a);
	}

	inline static int Rand(int a, int b)
    {
        return a + rand() % ((b - a) + 1);
    }

	template<typename T>
	inline static T Min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<typename T>
	inline static T Max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}
	 
	template<typename T>
	inline static T Lerp(const T& a, const T& b, float t)
	{
		return a + (b-a)*t;
	}

	template<typename T>
	inline static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x); 
	}

	inline static DirectX::XMVECTOR SphericalToCartesian(float radius, float theta, float phi)
	{
		return DirectX::XMVectorSet(
			radius*sinf(phi)*cosf(theta),
			radius*cosf(phi),
			radius*sinf(phi)*sinf(theta),
			1.0f);
	}

	inline static DirectX::XMMATRIX InverseTranspose(DirectX::CXMMATRIX M)
	{
		// Inverse-transpose is just applied to normals.  So zero out 
		// translation row so that it doesn't get into our inverse-transpose
		// calculation--we don't want the inverse-transpose of the translation.
        DirectX::XMMATRIX A = M;
        A.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

        DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
        return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, A));
	}

	inline static DirectX::XMFLOAT4X4 Identity4x4()
    {
        static DirectX::XMFLOAT4X4 I(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);

        return I;
    }

    
	inline static DirectX::XMMATRIX GetInverseMatrix(XMMATRIX Matrix)
	{
		XMVECTOR Determinant = XMMatrixDeterminant(Matrix);
		return XMMatrixInverse(&Determinant, Matrix);;
	}
};

