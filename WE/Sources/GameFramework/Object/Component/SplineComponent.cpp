#include "SplineComponent.h"
#include "Asset/AssetManager.h"
#include "Asset/SplineDataAsset.h"
#include "DirectX/DXMath.h"
#include "../World/World.h"
#include <algorithm>

void WSplineComponent::TickComponent_PostPhysics(float Delta)
{
	Super::TickComponent_PostPhysics(Delta);

	const XMFLOAT4 DebugColors[] = {
		{1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1},
		{1, 1, 0, 1}, {1, 0, 1, 1}, {0, 1, 1, 1}
	};
	
	int ColorSelector = 0;

	for (int i = 0; i < mSplineLUT.size() - 1; ++i)
	{
		XMFLOAT3 Start = GetWorldTransformAtDistanceAlongSpline(mSplineLUT[i].Distance).Translation;
		XMFLOAT3 End = GetWorldTransformAtDistanceAlongSpline(mSplineLUT[i + 1].Distance).Translation;
		GetWorld()->DrawDebugLine(Start, End, DebugColors[ColorSelector], 0);
		ColorSelector = (ColorSelector + 1) % 6;
	}
}

void WSplineComponent::AddSplineNode(const FSplineNode& Node)
{
	size_t NodeIndex = mSplineNodes.size();
	mSplineNodes.push_back(Node);

	if (NodeIndex == 0)
	{
		FSplineLUT LUT;
		LUT.Distance = 0;
		LUT.Location = Node.ControlPoint;
		LUT.Property1 = Node.Property1;
		LUT.Property2 = Node.Property2;
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

		FSampleParam Param;
		Param.t0 = 0.0f;
		Param.t1 = 1.0f;
		Param.Tolerance = 0.005f;
		Param.Pt0 = Pt0;
		Param.Pt1 = Pt1;
		Param.P0 = P0;
		Param.P1 = P1;
		Param.P2 = P2;
		Param.P3 = P3;

		Param.Property1_0 = mSplineNodes[NodeIndex - 1].Property1;
		Param.Property1_1 = mSplineNodes[NodeIndex].Property1;
		Param.Property2_0 = mSplineNodes[NodeIndex - 1].Property2;
		Param.Property2_1 = mSplineNodes[NodeIndex].Property2;

		AdaptiveSampleRecursive(Param, 0);

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
	XMFLOAT4X4 ComponentWorld = GetWorldFloat4x4();
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
	FSplineLUT LUT = GetSplineLUTAtDistanceAlongSpline(Distance);

	Transform.Translation = LUT.Location;
	Transform.Rotation = FDXMath::QuaternionToEuler(LUT.Quat);

	return Transform;
}

// TODO
FTransform WSplineComponent::GetWorldTransformAtDistanceAlongSpline(float Distance)
{
	XMMATRIX CW = GetWorldMatrix();

	FTransform SplineTransform = GetLocalTransformAtDistanceAlongSpline(Distance);
	XMMATRIX SM = SplineTransform.GetTransformMatrix();

	FTransform Result;
	Result.SetByTransformMatrix(SM * CW);

	return Result;
}

float WSplineComponent::GetCustomProperty1AtDistanceAlongSpline(float Distance)
{
	return GetSplineLUTAtDistanceAlongSpline(Distance).Property1;
}

float WSplineComponent::GetCustomProperty2AtDistanceAlongSpline(float Distance)
{
	return GetSplineLUTAtDistanceAlongSpline(Distance).Property2;
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

WSplineComponent::FSplineLUT WSplineComponent::GetSplineLUTAtDistanceAlongSpline(float Distance)
{
	if (mSplineLUT.empty())
	{
		return FSplineLUT();
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
		return *Iter;
	}

	// 4. 보간 처리 (it는 Distance보다 큰 첫 번째 점, prevIt은 Distance보다 작은 마지막 점)
	auto PrevIter = std::prev(Iter);

	float LeftDist = PrevIter->Distance;
	float RightDist = Iter->Distance;

	// 분모가 0이 되는 것 방지 (매우 가까운 노드 대응)
	float Delta = RightDist - LeftDist;
	float Alpha = (Delta > 0.00001f) ? (Distance - LeftDist) / Delta : 0.0f;

	FSplineLUT LUT;

	XMVECTOR P0 = XMLoadFloat3(&PrevIter->Location);
	XMVECTOR P1 = XMLoadFloat3(&Iter->Location);
	XMVECTOR L = XMVectorLerp(P0, P1, Alpha);
	XMStoreFloat3(&LUT.Location , L);


	XMVECTOR Q0 = XMLoadFloat4(&PrevIter->Quat);
	XMVECTOR Q1 = XMLoadFloat4(&Iter->Quat);
	XMVECTOR Q = XMQuaternionSlerp(Q0, Q1, Alpha);
	XMStoreFloat4(&LUT.Quat, Q);

	LUT.Distance = Distance;
	LUT.Property1 = FDXMath::Lerp(PrevIter->Property1, Iter->Property1, Alpha);
	LUT.Property2 = FDXMath::Lerp(PrevIter->Property2, Iter->Property2, Alpha);

	return LUT;
}

void WSplineComponent::AdaptiveSampleRecursive(const FSampleParam& Param, UINT Depth)
{
	float tMid = (Param.t0 + Param.t1) / 2;

	XMVECTOR PtMid = FDXMath::CalculateCubicBezier(Param.P0, Param.P1, Param.P2, Param.P3, tMid);
	XMVECTOR PMid = XMVectorDivide(XMVectorAdd(Param.Pt0, Param.Pt1), XMVectorReplicate(2.0f));

	float Distance = XMVectorGetX(XMVector3Length(XMVectorSubtract(PtMid, PMid)));

	if (Distance > Param.Tolerance && Depth < 10)
	{
		float Property1Mid = (Param.Property1_0 + Param.Property1_1) / 2.0f;
		float Property2Mid = (Param.Property2_0 + Param.Property2_1) / 2.0f;

		FSampleParam LeftParam = Param;
		LeftParam.t1 = tMid;
		LeftParam.Pt1 = PtMid;
		LeftParam.Property1_1 = Property1Mid;
		LeftParam.Property2_1 = Property2Mid;
		AdaptiveSampleRecursive(LeftParam, Depth + 1);
		FSampleParam RightParam = Param;
		RightParam.t0 = tMid;
		RightParam.Pt0 = PtMid;
		RightParam.Property1_0 = Property1Mid;
		RightParam.Property2_0 = Property2Mid;
		AdaptiveSampleRecursive(RightParam, Depth + 1);
	}
	else
	{
		FSplineLUT LUT;
		XMStoreFloat3(&LUT.Location, Param.Pt1);
		LUT.Distance = mSplineLUT.back().Distance +  XMVector3Length(XMVectorSubtract(Param.Pt1, Param.Pt0)).m128_f32[0];

		XMVECTOR Tangent = XMVector3Normalize(Param.Pt1 - Param.Pt0); // 진행 방향
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

		LUT.Property1 = Param.Property1_1;
		LUT.Property2 = Param.Property2_1;

		mSplineLUT.push_back(LUT);
	}
}
