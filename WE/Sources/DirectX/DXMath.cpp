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

bool FTransform::Equal(const FTransform& Another, float Epsilon)
{
	// XMFLOAT3를 XMVECTOR로 로드
	XMVECTOR s1 = XMLoadFloat3(&Scale);
	XMVECTOR r1 = XMLoadFloat3(&Rotation);
	XMVECTOR t1 = XMLoadFloat3(&Translation);

	XMVECTOR s2 = XMLoadFloat3(&Another.Scale);
	XMVECTOR r2 = XMLoadFloat3(&Another.Rotation);
	XMVECTOR t2 = XMLoadFloat3(&Another.Translation);

	// 모든 성분이 거의 일치하는지 확인 (Epsilon 값 조절 가능)
	return XMVector3NearEqual(s1, s2, XMVectorReplicate(Epsilon)) &&
		XMVector3NearEqual(r1, r2, XMVectorReplicate(Epsilon)) &&
		XMVector3NearEqual(t1, t2, XMVectorReplicate(Epsilon));
}

void FTransform::SetRotationByQuat(XMFLOAT4 QuatRotation)
{
	Rotation = FDXMath::QuaternionToEuler(QuatRotation);
}

XMMATRIX XM_CALLCONV FTransform::GetScaleMatrix() const
{
	return XMMatrixScalingFromVector(XMLoadFloat3(&Scale));
}

XMMATRIX XM_CALLCONV FTransform::GetRotationMatrix() const
{
	return XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(Rotation.x),
		XMConvertToRadians(Rotation.y),
		XMConvertToRadians(Rotation.z)
	);
}

XMMATRIX XM_CALLCONV FTransform::GetTranslationMatrix() const
{
	return XMMatrixTranslationFromVector(XMLoadFloat3(&Translation));
}

XMVECTOR XM_CALLCONV FTransform::GetTranslationVector() const
{
	return XMLoadFloat3(&Translation);
}

XMMATRIX XM_CALLCONV FTransform::GetTransformMatrix() const
{
	return GetScaleMatrix() * GetRotationMatrix() * GetTranslationMatrix();
}

void XM_CALLCONV FTransform::SetByTransformMatrix(FXMMATRIX M)
{
	XMVECTOR S;
	XMVECTOR RQ;
	XMVECTOR T;
	XMMatrixDecompose(&S, &RQ, &T, M);
	XMStoreFloat3(&Scale, S);
	XMStoreFloat3(&Translation, T);
	XMFLOAT4 RotationQuat;
	XMStoreFloat4(&RotationQuat, RQ);
	SetRotationByQuat(RotationQuat);
}

XMFLOAT4X4 FTransform::GetScaleFloat4x4() const
{
	XMMATRIX S = GetScaleMatrix();
	XMFLOAT4X4 Mat;
	XMStoreFloat4x4(&Mat, S);
	return Mat;
}

XMFLOAT4X4 FTransform::GetRotationFloat4x4() const
{
	XMMATRIX R = GetRotationMatrix();
	XMFLOAT4X4 Mat;
	XMStoreFloat4x4(&Mat, R);
	return Mat;
}

XMFLOAT4X4 FTransform::GetTranslationFloat4x4() const
{
	XMMATRIX T = GetTranslationMatrix();
	XMFLOAT4X4 Mat;
	XMStoreFloat4x4(&Mat, T);
	return Mat;
}

XMFLOAT4X4 FTransform::GetTransformFloat4x4() const
{
	XMFLOAT4X4 Matrix;
	XMStoreFloat4x4(&Matrix, GetTransformMatrix());
	return Matrix;
}

XMFLOAT4 FTransform::GetQuaternionRotationFloat4() const
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

XMFLOAT4X4 FTransform::GetQuaternionRotationFloat4x4() const
{
	XMFLOAT4 Quat = GetQuaternionRotationFloat4();
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

XMFLOAT3 XM_CALLCONV FDXMath::GetEulerRotationFromVectors(FXMVECTOR F, FXMVECTOR R, FXMVECTOR U)
{
	XMFLOAT3 Forward;
	XMFLOAT3 Right;
	XMFLOAT3 Up;
	XMStoreFloat3(&Forward, F);
	XMStoreFloat3(&Right, R);
	XMStoreFloat3(&Up, U);

	float Pitch = -asinf(Forward.y);
	float Yaw = atan2f(Forward.x, Forward.z);
	float Roll = atan2f(Right.y, Up.y);

	return XMFLOAT3(
		XMConvertToDegrees(Pitch),
		XMConvertToDegrees(Yaw),
		XMConvertToDegrees(Roll)
	);
}

XMFLOAT3 FDXMath::QuaternionToEuler(XMFLOAT4 q)
{
	XMFLOAT3 e;

	float x = q.x;
	float y = q.y;
	float z = q.z;
	float w = q.w;

	// Pitch (X-axis)
	float sinp = 2.0f * (w * x - y * z);
	sinp = Clamp(sinp, -1.0f, 1.0f);
	e.x = std::asin(sinp);

	// Yaw (Y-axis)
	e.y = std::atan2(
		2.0f * (w * y + x * z),
		1.0f - 2.0f * (x * x + y * y)
	);

	// Roll (Z-axis)
	e.z = std::atan2(
		2.0f * (w * z + x * y),
		1.0f - 2.0f * (x * x + z * z)
	);

	// Radian -> Degree 변환
	e.x = XMConvertToDegrees(e.x);
	e.y = XMConvertToDegrees(e.y);
	e.z = XMConvertToDegrees(e.z);

	return e;
}

XMVECTOR XM_CALLCONV FDXMath::EulerToQuaternionVector(const XMFLOAT3& Euler)
{
	XMVECTOR Q = XMQuaternionRotationRollPitchYaw(Euler.x, Euler.y ,Euler.z);
	return Q;
}

XMFLOAT4 FDXMath::EulerToQuaternion(XMFLOAT3 Euler)
{
	XMFLOAT4 Quat;
	XMStoreFloat4(&Quat, EulerToQuaternionVector(Euler));
	return Quat;
}

XMVECTOR XM_CALLCONV FDXMath::CalculateCubicBezier(FXMVECTOR P0, FXMVECTOR P1, FXMVECTOR P2, GXMVECTOR P3, float t)
{
	// t가 0에서 1 사이인지 보장
	t = XMMin(XMMax(t, 0.0f), 1.0f);

	float InvT = 1.0f - t;

	// 베지에 공식: B(t) = (1-t)^3*P0 + 3(1-t)^2*t*P1 + 3(1-t)t^2*P2 + t^3*P3
	float b0 = InvT * InvT * InvT;
	float b1 = 3.0f * InvT * InvT * t;
	float b2 = 3.0f * InvT * t * t;
	float b3 = t * t * t;

	// DirectXMath의 가중치 합 연산 사용
	XMVECTOR Result = XMVectorScale(P0, b0);
	Result = XMVectorMultiplyAdd(XMVectorScale(P1, b1), XMVectorReplicate(1.0f), Result);
	Result = XMVectorMultiplyAdd(XMVectorScale(P2, b2), XMVectorReplicate(1.0f), Result);
	Result = XMVectorMultiplyAdd(XMVectorScale(P3, b3), XMVectorReplicate(1.0f), Result);

	return Result;
}

XMVECTOR XM_CALLCONV FDXMath::CalculateCubicBezierForward(FXMVECTOR P0, FXMVECTOR P1, FXMVECTOR P2, GXMVECTOR P3, float t)
{
	float invT = 1.0f - t;

	// 3차 베지에의 1차 도함수 공식
	// B'(t) = 3(1-t)^2(P1-P0) + 6(1-t)t(P2-P1) + 3t^2(P3-P2)
	float b0 = 3.0f * invT * invT;
	float b1 = 6.0f * invT * t;
	float b2 = 3.0f * t * t;

	XMVECTOR T0 = XMVectorSubtract(P1, P0);
	XMVECTOR T1 = XMVectorSubtract(P2, P1);
	XMVECTOR T2 = XMVectorSubtract(P3, P2);

	XMVECTOR tangent = XMVectorScale(T0, b0);
	tangent = XMVectorAdd(XMVectorScale(T1, b1), tangent);
	tangent = XMVectorAdd(XMVectorScale(T2, b2), tangent);

	return XMVector3Normalize(tangent); // 정규화 필수
}

bool FDXMath::Equal(const XMFLOAT3& A, const XMFLOAT3& B, float Epsilon)
{
	XMVECTOR V1 = XMLoadFloat3(&A);
	XMVECTOR V2 = XMLoadFloat3(&B);
	return XMVector3NearEqual(V1, V2, XMVectorReplicate(Epsilon));
}
