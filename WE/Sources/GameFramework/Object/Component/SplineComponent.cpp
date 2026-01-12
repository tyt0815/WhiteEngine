#include "SplineComponent.h"
#include "Asset/AssetManager.h"
#include "Asset/SplineDataAsset.h"
#include "DirectX/DXMath.h"

void WSplineComponent::AddSplineNode(const FSplineNode& Node)
{
	mSplineNodes.push_back(Node);
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
	for (size_t i = 0; i < DataSize; ++i)
	{
		FSplineNode Node;
		Node.ControlPoint = SplineDataAsset->ControlPoints[i];
		Node.LeftHandle = SplineDataAsset->LeftHandles[i];
		Node.RightHandle = SplineDataAsset->RightHandles[i];
		Node.Property1 = SplineDataAsset->Property1[i];
		Node.Property2 = SplineDataAsset->Property2[i];
		mSplineNodes.push_back(Node);
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
		/*U = XMVector3Normalize(XMVector3Cross(F, XMVectorSet(1, 0, 0, 0)));
		R = XMVector3Cross(U, F);*/
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
