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

XMMATRIX FTransform::GetTransformMatrix()
{
	XMMATRIX S = XMMatrixScalingFromVector(GetScaleXMVECTOR());
	XMMATRIX R =
		XMMatrixRotationX(XMConvertToRadians(Rotation.x)) *
		XMMatrixRotationY(XMConvertToRadians(Rotation.y)) *
		XMMatrixRotationZ(XMConvertToRadians(Rotation.z));
	XMMATRIX T = XMMatrixTranslationFromVector(GetTranslationXMVECTOR());
	return S * R * T;
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