#include "SplineComponent.h"
#include "Asset/AssetManager.h"
#include "Asset/SplineDataAsset.h"
#include "DirectX/DXMath.h"
#include <algorithm>

void WSplineComponent::AddSplineNode(const FSplineNode& Node)
{
	size_t NodeIndex = mSplineNodes.size();
	mSplineNodes.push_back(Node);

	if (NodeIndex == 0)
	{
		FSplineLUT LUT;
		LUT.Distance = 0;
		LUT.Location = Node.ControlPoint;
		mSplineLUT.push_back(LUT);
	}
	else
	{
		XMVECTOR P0 = XMLoadFloat3(&mSplineNodes[NodeIndex - 1].ControlPoint);
		XMVECTOR P1 = XMLoadFloat3(&mSplineNodes[NodeIndex - 1].RightHandle);
		XMVECTOR P2 = XMLoadFloat3(&mSplineNodes[NodeIndex].LeftHandle);
		XMVECTOR P3 = XMLoadFloat3(&mSplineNodes[NodeIndex].ControlPoint);

		XMVECTOR Pt0 = FDXMath::CalculateCubicBezier(P0, P1, P2, P3, 0);
		XMVECTOR Pt1 = FDXMath::CalculateCubicBezier(P0, P1, P2, P3, 1);
		AdaptiveSampleRecursive(0.0f, 1.0f, 0.005f, Pt0, Pt1, P0, P1, P2, P3, 0);

		if (NodeIndex == 1)
		{
			mSplineLUT[0].Quat = mSplineLUT[1].Quat;
		}
	}
}

void WSplineComponent::LoadSplineFromAsset(const std::wstring& AssetName)
{
	FSplineDataAsset* SplineDataAsset = FAssetManager::GetAsset<FSplineDataAsset>(AssetName);
	if (SplineDataAsset == nullptr)
	{
		return;
	}
	size_t DataSize = SplineDataAsset->ControlPoints.size();

	mSplineNodes.clear();
	mSplineLUT.clear();
	for (size_t i = 0; i < DataSize; ++i)
	{
		FSplineNode Node;
		Node.ControlPoint = SplineDataAsset->ControlPoints[i];
		Node.LeftHandle = SplineDataAsset->LeftHandles[i];
		Node.RightHandle = SplineDataAsset->RightHandles[i];
		Node.Property1 = SplineDataAsset->Property1[i];
		Node.Property2 = SplineDataAsset->Property2[i];
		AddSplineNode(Node);
	}
}

XMFLOAT3 WSplineComponent::GetLocalLocationAtSplineInputKey(float InputKey)
{
	XMVECTOR P0;
	XMVECTOR P1;
	XMVECTOR P2;
	XMVECTOR P3;
	float t;
	SelectBezierPointsByInputKey(InputKey, &P0, &P1, &P2, &P3, t);

	XMVECTOR P = FDXMath::CalculateCubicBezier(P0, P1, P2, P3, t);

	XMFLOAT3 Location;
	XMStoreFloat3(&Location, P);
	return Location;
}

XMFLOAT3 WSplineComponent::GetLocalRotationAtSplineInputKey(float InputKey)
{
	XMVECTOR P0;
	XMVECTOR P1;
	XMVECTOR P2;
	XMVECTOR P3;
	float t;

	SelectBezierPointsByInputKey(InputKey, &P0, &P1, &P2, &P3, t);

	XMVECTOR F = FDXMath::CalculateCubicBezierForward(P0, P1, P2, P3, t);
	XMVECTOR U;
	XMVECTOR R;
	if (XMVector3Dot(F, XMVectorSet(0, 1, 0, 0)).m128_f32[0] > 0.9)
	{
		R = XMVector3Normalize(XMVector3Cross(F, XMVectorSet(0, 1, 0, 0)));
		U = XMVector3Cross(F, R);
	}
	else
	{
		R = XMVector3Normalize(XMVector3Cross(XMVectorSet(0, 1, 0, 0), F));
		U = XMVector3Cross(F, R);
	}
	
	XMFLOAT3 Rotation = FDXMath::GetEulerRotationFromVectors(F, R, U);
	return Rotation;
}

FTransform WSplineComponent::GetLocalTransformAtSplineInputKey(float InputKey)
{
	FTransform Transform;
	Transform.Translation = GetLocalLocationAtSplineInputKey(InputKey);
	Transform.Rotation = GetLocalRotationAtSplineInputKey(InputKey);

	return Transform;
}

FTransform WSplineComponent::GetWorldTransformAtSplineInputKey(float InputKey)
{
	XMFLOAT4X4 ComponentWorld = GetWorldMatrix();
	XMMATRIX CW = XMLoadFloat4x4(&ComponentWorld);

	FTransform SplineTransform = GetLocalTransformAtSplineInputKey(InputKey);
	XMMATRIX SM = SplineTransform.GetTransformMatrix();

	FTransform Result;
	Result.SetByTransformMatrix(SM * CW);

	return Result;
}

XMFLOAT3 WSplineComponent::GetLocalLocationAtDistanceAlongSpline(float Distance)
{
	return GetLocalTransformAtDistanceAlongSpline(Distance).Translation;
}

XMFLOAT3 WSplineComponent::GetLocalRotationAtDistanceAlongSpline(float Distance)
{
	return GetLocalTransformAtDistanceAlongSpline(Distance).Rotation;
}

FTransform WSplineComponent::GetLocalTransformAtDistanceAlongSpline(float Distance)
{
	FTransform Transform;
	if (mSplineLUT.empty())
	{
		return Transform;
	}

	// 1. 범위 클램핑
	Distance = FDXMath::Clamp(Distance, 0.0f, mSplineLUT.back().Distance);

	// 2. 이진 탐색으로 이터레이터 찾기
	auto Iter = std::lower_bound(mSplineLUT.begin(), mSplineLUT.end(), Distance,
		[](const FSplineLUT& LUT, float Value) { return LUT.Distance < Value; }
	);

	// 3. 예외 처리: 시작점이거나 정확히 일치하는 경우
	if (Iter == mSplineLUT.begin())
	{
		Transform.Translation = Iter->Location;
		Transform.Rotation = FDXMath::QuaternionToEuler(Iter->Quat);
		return Transform;
	}

	// 4. 보간 처리 (it는 Distance보다 큰 첫 번째 점, prevIt은 Distance보다 작은 마지막 점)
	auto PrevIter = std::prev(Iter);

	float LeftDist = PrevIter->Distance;
	float RightDist = Iter->Distance;

	// 분모가 0이 되는 것 방지 (매우 가까운 노드 대응)
	float Delta = RightDist - LeftDist;
	float Alpha = (Delta > 0.00001f) ? (Distance - LeftDist) / Delta : 0.0f;

	XMVECTOR P0 = XMLoadFloat3(&PrevIter->Location);
	XMVECTOR P1 = XMLoadFloat3(&Iter->Location);
	XMVECTOR L = XMVectorLerp(P0, P1, Alpha);
	XMStoreFloat3(&Transform.Translation, L);

	
	XMVECTOR Q0 = XMLoadFloat4(&PrevIter->Quat);
	XMVECTOR Q1 = XMLoadFloat4(&Iter->Quat);
	XMVECTOR Q = XMQuaternionSlerp(Q0, Q1, Alpha);
	XMFLOAT4 Quat;
	XMStoreFloat4(&Quat, Q);
	Transform.Rotation = FDXMath::QuaternionToEuler(Quat);

	return Transform;
}

// TODO
FTransform WSplineComponent::GetWorldTransformAtDistanceAlongSpline(float Distance)
{
	XMFLOAT4X4 ComponentWorld = GetWorldMatrix();
	XMMATRIX CW = XMLoadFloat4x4(&ComponentWorld);

	FTransform SplineTransform = GetLocalTransformAtDistanceAlongSpline(Distance);
	XMMATRIX SM = SplineTransform.GetTransformMatrix();

	FTransform Result;
	Result.SetByTransformMatrix(SM * CW);

	return Result;
}

void WSplineComponent::SelectSplineNodesByInputKey(float InputKey, FSplineNode& LeftNode, FSplineNode& RightNode, float& t)
{
	if (mSplineNodes.size() < 2)
	{
		return;
	}

	int NodeIndex = FDXMath::Clamp<int>(static_cast<int>(InputKey), 0, static_cast<int>(mSplineNodes.size() - 2));
	t = FDXMath::Clamp<float>(InputKey - NodeIndex, 0, 1);

	LeftNode = mSplineNodes[NodeIndex];
	RightNode = mSplineNodes[NodeIndex + 1];
}

void WSplineComponent::SelectBezierPointsByInputKey(float InputKey, XMVECTOR* P0, XMVECTOR* P1, XMVECTOR* P2, XMVECTOR* P3, float& t)
{
	FSplineNode LeftNode;
	FSplineNode RightNode;
	SelectSplineNodesByInputKey(InputKey, LeftNode, RightNode, t);
	*P0 = XMLoadFloat3(&LeftNode.ControlPoint);
	*P1 = XMLoadFloat3(&LeftNode.RightHandle);
	*P2 = XMLoadFloat3(&RightNode.LeftHandle);
	*P3 = XMLoadFloat3(&RightNode.ControlPoint);
}

void XM_CALLCONV WSplineComponent::AdaptiveSampleRecursive(
	float t0, float t1, float Tolerance,
	FXMVECTOR Pt0, FXMVECTOR Pt1,
	FXMVECTOR P0, GXMVECTOR P1, HXMVECTOR P2, HXMVECTOR P3,
	UINT Depth
)
{
	float tMid = (t0 + t1) / 2;

	XMVECTOR PtMid = FDXMath::CalculateCubicBezier(P0, P1, P2, P3, tMid);
	XMVECTOR PMid = XMVectorDivide(XMVectorAdd(Pt0, Pt1), XMVectorReplicate(2.0f));

	float Distance = XMVector3Length(XMVectorSubtract(PtMid, PMid)).m128_f32[0];

	if (Distance > Tolerance && Depth < 10)
	{
		AdaptiveSampleRecursive(t0, tMid, Tolerance, Pt0, PtMid, P0, P1, P2, P3, Depth + 1);
		AdaptiveSampleRecursive(tMid, t1, Tolerance, PtMid, Pt1, P0, P1, P2, P3, Depth + 1);
	}
	else
	{
		FSplineLUT LUT;
		XMStoreFloat3(&LUT.Location, Pt1);
		LUT.Distance = mSplineLUT.back().Distance +  XMVector3Length(XMVectorSubtract(Pt1, Pt0)).m128_f32[0];

		XMVECTOR Tangent = XMVector3Normalize(Pt1 - Pt0); // 진행 방향
		XMVECTOR Up = XMVectorSet(0, 1, 0, 0);
		XMVECTOR Right;
		if (XMVector3Dot(Tangent, Up).m128_f32[0] > 0.9f)
		{
			Right = XMVector3Normalize(XMVector3Cross(Tangent, XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)));
		}
		else
		{
			Right = XMVector3Normalize(XMVector3Cross(Up, Tangent));
		}
		 
		Up = XMVector3Cross(Tangent, Right); // 실제 수직 벡터 재계산

		XMMATRIX RotMat;
		RotMat.r[0] = Right;
		RotMat.r[1] = Up;
		RotMat.r[2] = Tangent;
		RotMat.r[3] = XMVectorSet(0, 0, 0, 1);

		XMStoreFloat4(&LUT.Quat, XMQuaternionRotationMatrix(RotMat));

		mSplineLUT.push_back(LUT);
	}
}
