//***************************************************************************************
// MathHelper.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "DXMath.h"
#include <float.h>
#include <cmath>

using namespace DirectX;

const float FDXMath::Infinity = FLT_MAX;
const float FDXMath::Pi       = 3.1415926535f;

const FTransform FTransform::Zeros = {
	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f}
};

const FTransform FTransform::Default = {
	{1.0f, 1.0f, 1.0f},
	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f}
};

FTransform::FTransform(XMFLOAT3 InScale, XMFLOAT3 InRotation, XMFLOAT3 InTranslation) :
	Scale(InScale),
	Rotation(InRotation),
	Translation(InTranslation)
{
}

XMFLOAT4X4 FTransform::GetScaleMatrix()
{
	XMFLOAT4X4 Mat;
	XMStoreFloat4x4(&Mat, XMMatrixScalingFromVector(XMLoadFloat3(&Scale)));
	return Mat;
}

XMFLOAT4X4 FTransform::GetRotationMatrix()
{
	XMMATRIX R = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(Rotation.x),
		XMConvertToRadians(Rotation.y),
		XMConvertToRadians(Rotation.z)
	);
	XMFLOAT4X4 Mat;
	XMStoreFloat4x4(&Mat, R);
	return Mat;
}

XMFLOAT4X4 FTransform::GetTranslationMatrix()
{
	XMFLOAT4X4 Mat;
	XMStoreFloat4x4(&Mat, XMMatrixTranslationFromVector(XMLoadFloat3(&Translation)));
	return Mat;
}

XMFLOAT4X4 FTransform::GetTransformMatrix()
{
	XMFLOAT4X4 Scale = GetScaleMatrix();
	XMFLOAT4X4 Rotation = GetRotationMatrix();
	XMFLOAT4X4 Translation = GetTranslationMatrix();
	XMMATRIX S = XMLoadFloat4x4(&Scale);
	XMMATRIX R = XMLoadFloat4x4(&Rotation);
	XMMATRIX T = XMLoadFloat4x4(&Translation);
	XMFLOAT4X4 Matrix;
	XMStoreFloat4x4(&Matrix, S * R * T);
	return Matrix;
}

XMFLOAT4 FTransform::GetQuaternionRotation()
{
	XMVECTOR RotationQuat = XMQuaternionRotationRollPitchYaw(
		XMConvertToRadians(Rotation.x),
		XMConvertToRadians(Rotation.y),
		XMConvertToRadians(Rotation.z)
	);
	XMFLOAT4 Quat;
	XMStoreFloat4(&Quat, RotationQuat);
	return Quat;
}

XMFLOAT4X4 FTransform::GetQuaternionRotationMatrix()
{
	XMFLOAT4 Quat = GetQuaternionRotation();
	XMFLOAT4X4 Mat;
	XMStoreFloat4x4(&Mat, XMMatrixRotationQuaternion(XMLoadFloat4(&Quat)));
	return Mat;
}

float FDXMath::AngleFromXY(float x, float y)
{
	float theta = 0.0f;
 
	// Quadrant I or IV
	if(x >= 0.0f) 
	{
		// If x = 0, then atanf(y/x) = +pi/2 if y > 0
		//                atanf(y/x) = -pi/2 if y < 0
		theta = atanf(y / x); // in [-pi/2, +pi/2]

		if(theta < 0.0f)
			theta += 2.0f*Pi; // in [0, 2*pi).
	}

	// Quadrant II or III
	else      
		theta = atanf(y/x) + Pi; // in [0, 2*pi).

	return theta;
}

XMVECTOR FDXMath::RandUnitVec3()
{
	XMVECTOR One  = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();

	// Keep trying until we get a point on/in the hemisphere.
	while(true)
	{
		// Generate random point in the cube [-1,1]^3.
		XMVECTOR v = XMVectorSet(FDXMath::RandF(-1.0f, 1.0f), FDXMath::RandF(-1.0f, 1.0f), FDXMath::RandF(-1.0f, 1.0f), 0.0f);

		// Ignore points outside the unit sphere in order to get an even distribution 
		// over the unit sphere.  Otherwise points will clump more on the sphere near 
		// the corners of the cube.

		if( XMVector3Greater( XMVector3LengthSq(v), One) )
			continue;

		return XMVector3Normalize(v);
	}
}

XMVECTOR FDXMath::RandHemisphereUnitVec3(XMVECTOR n)
{
	XMVECTOR One  = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();

	// Keep trying until we get a point on/in the hemisphere.
	while(true)
	{
		// Generate random point in the cube [-1,1]^3.
		XMVECTOR v = XMVectorSet(FDXMath::RandF(-1.0f, 1.0f), FDXMath::RandF(-1.0f, 1.0f), FDXMath::RandF(-1.0f, 1.0f), 0.0f);

		// Ignore points outside the unit sphere in order to get an even distribution 
		// over the unit sphere.  Otherwise points will clump more on the sphere near 
		// the corners of the cube.
		
		if( XMVector3Greater( XMVector3LengthSq(v), One) )
			continue;

		// Ignore points in the bottom hemisphere.
		if( XMVector3Less( XMVector3Dot(n, v), Zero ) )
			continue;

		return XMVector3Normalize(v);
	}
}

DirectX::XMFLOAT4X4 FDXMath::CalcViewMatrix(DirectX::XMFLOAT3 Target, DirectX::XMFLOAT3 Up, DirectX::XMFLOAT3 Position)
{
	XMVECTOR U = XMLoadFloat3(&Up);
	XMVECTOR T = XMLoadFloat3(&Target);
	XMVECTOR P = XMLoadFloat3(&Position);
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(T, P));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(U, L));
	U = XMVector3Normalize(XMVector3Cross(L, R));

	// Fill in the view matrix entries.
	float x = -XMVectorGetX(XMVector3Dot(P, R));
	float y = -XMVectorGetX(XMVector3Dot(P, U));
	float z = -XMVectorGetX(XMVector3Dot(P, L));

	XMFLOAT4X4 View = FDXMath::Identity4x4();
	XMFLOAT3 Right;
	XMFLOAT3 Look;
	XMStoreFloat3(&Right, R);
	XMStoreFloat3(&Up, U);
	XMStoreFloat3(&Look, L);
	View(0, 0) = Right.x;
	View(1, 0) = Right.y;
	View(2, 0) = Right.z;
	View(3, 0) = x;

	View(0, 1) = Up.x;
	View(1, 1) = Up.y;
	View(2, 1) = Up.z;
	View(3, 1) = y;

	View(0, 2) = Look.x;
	View(1, 2) = Look.y;
	View(2, 2) = Look.z;
	View(3, 2) = z;

	View(0, 3) = 0.0f;
	View(1, 3) = 0.0f;
	View(2, 3) = 0.0f;
	View(3, 3) = 1.0f;

	return View;
}
