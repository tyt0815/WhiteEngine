#include "SceneComponent.h"
#include "Actor/Actor.h"

WSceneComponent::WSceneComponent()
{
}

void WSceneComponent::ActivateWithChild()
{
	if (!IsActivate())
	{
		Activate();
	}

	for (auto& Child : mChilds)
	{
		Child.lock()->ActivateWithChild();
	}
}

void WSceneComponent::DeactivateWithChild()
{
	if (IsActivate())
	{
		Deactivate();
	}

	for (auto& Child : mChilds)
	{
		Child.lock()->DeactivateWithChild();
	}
}

void WSceneComponent::UpdateWorldMatrix()
{
	if (mbWorldFloat4x4Dirty)
	{
		XMMATRIX L = mTransform.GetTransformMatrix(); // 내 로컬 행렬
		XMMATRIX W;

		if (auto Parent = mParent.lock())
		{
			// 부모의 WorldMatrix를 가져옴 (재귀)
			XMMATRIX PW = Parent->GetWorldMatrix();
			// World = Local * ParentWorld
			W = XMMatrixMultiply(L, PW);
		}
		else
		{
			W = L;
		}

		// 결과 저장 및 역행렬 미리 계산
		XMMATRIX InvW = FDXMath::GetInverseMatrix(W);
		XMStoreFloat4x4(&mWorldFloat4x4, W);
		XMStoreFloat4x4(&mInvWorldFloat4x4, InvW);

		mbWorldFloat4x4Dirty = false;
	}
}

DirectX::XMFLOAT4X4 WSceneComponent::GetWorldFloat4x4()
{
	UpdateWorldMatrix();
	return mWorldFloat4x4;
}

DirectX::XMFLOAT4X4 WSceneComponent::GetInverseWorldFloat4x4()
{
	UpdateWorldMatrix();
	return mInvWorldFloat4x4;
}

DirectX::XMMATRIX XM_CALLCONV WSceneComponent::GetWorldMatrix()
{
	UpdateWorldMatrix();
	DirectX::XMMATRIX M = XMLoadFloat4x4(&mWorldFloat4x4);
	return M;
}

DirectX::XMMATRIX XM_CALLCONV WSceneComponent::GetInverseWorldMatrix()
{
	UpdateWorldMatrix();
	DirectX::XMMATRIX M = XMLoadFloat4x4(&mInvWorldFloat4x4);
	return M;
}

void WSceneComponent::UpdateRecursive()
{
	if (IsActivate())
	{
		Update();
	}

	for (auto& ChildWeak : mChilds)
	{
		if (auto Child = ChildWeak.lock())
		{
			Child->UpdateRecursive();
		}
	}
}

void WSceneComponent::SetupAttachment(WSceneComponent* InParent)
{
	if (InParent)
	{
		mParent = InParent->GetWeakPtr<WSceneComponent>();

		if (auto Parent = mParent.lock())
		{
			Parent->mChilds.push_back(GetWeakPtr<WSceneComponent>());
		}
	}
}

DirectX::XMFLOAT4 WSceneComponent::GetLocalQuatRotation()
{
	DirectX::XMVECTOR LocalEulerRadian = XMVectorSet(
		DirectX::XMConvertToRadians(mTransform.Rotation.x),
		DirectX::XMConvertToRadians(mTransform.Rotation.y),
		DirectX::XMConvertToRadians(mTransform.Rotation.z),
		0.0f
	);

	DirectX::XMVECTOR LocalQuat = XMQuaternionRotationRollPitchYawFromVector(LocalEulerRadian);
	DirectX::XMFLOAT4 LocalQuatRotation;
	DirectX::XMStoreFloat4(&LocalQuatRotation, LocalQuat);
	return LocalQuatRotation;
}

DirectX::XMFLOAT4 WSceneComponent::GetWorldQuatRotation()
{
	if (auto Parent = mParent.lock())
	{
		DirectX::XMFLOAT4 ParentWorldQuatRotation = Parent->GetWorldQuatRotation();
		DirectX::XMFLOAT4 LocalQuatRotation = GetLocalQuatRotation();
		DirectX::XMVECTOR ParentWorldQuat = XMLoadFloat4(&ParentWorldQuatRotation);
		DirectX::XMVECTOR LocalQuat = DirectX::XMLoadFloat4(&LocalQuatRotation);
		DirectX::XMVECTOR WorldQuat = DirectX::XMQuaternionMultiply(LocalQuat, ParentWorldQuat);
		DirectX::XMFLOAT4 WorldQuatRotation;
		DirectX::XMStoreFloat4(&WorldQuatRotation, WorldQuat);
		return WorldQuatRotation;
	}
	else
	{
		return GetLocalQuatRotation();
	}
}

DirectX::XMFLOAT3 WSceneComponent::GetWorldLocation()
{
	return GetWorldTransform().Translation;
}

DirectX::XMFLOAT3 WSceneComponent::GetWorldRotation()
{
	return GetWorldTransform().Rotation;
}

DirectX::XMFLOAT3 WSceneComponent::GetWorldScale()
{
	return GetWorldTransform().Scale;
}

XMFLOAT3 WSceneComponent::GetForwardVector()
{
	XMMATRIX RotationMatrix = mTransform.GetRotationMatrix();
	XMVECTOR VForward = XMVector3Transform({ 0.0f, 0.0f, 1.0f }, RotationMatrix);
	XMFLOAT3 Foward;
	XMStoreFloat3(&Foward, XMVector3Normalize(VForward));
	return Foward;
}

XMFLOAT3 WSceneComponent::GetRightVector()
{
	XMMATRIX RotationMatrix = mTransform.GetRotationMatrix();
	XMVECTOR VRight = XMVector3Transform({ 1.0f, 0.0f, 0.0f }, RotationMatrix);
	XMFLOAT3 Right;
	XMStoreFloat3(&Right, XMVector3Normalize(VRight));
	return Right;
}

XMFLOAT3 WSceneComponent::GetUpVector()
{
	XMMATRIX RotationMatrix = mTransform.GetRotationMatrix();
	XMVECTOR VUp = XMVector3Transform({ 0.0f, 1.0f, 0.0f }, RotationMatrix);
	XMFLOAT3 Up;
	XMStoreFloat3(&Up, XMVector3Normalize(VUp));
	return Up;
}

FTransform WSceneComponent::GetWorldTransform()
{
	XMVECTOR S;
	XMVECTOR RQ;
	XMVECTOR T;
	XMMATRIX W = GetWorldMatrix();
	XMMatrixDecompose(&S, &RQ, &T, W);
	FTransform Transform;
	XMStoreFloat3(&Transform.Scale, S);
	XMStoreFloat3(&Transform.Translation, T);
	XMFLOAT4 Quat;
	XMStoreFloat4(&Quat, RQ);
	Transform.SetRotationByQuat(Quat);
	return Transform;
}

void WSceneComponent::SetLocalRotation(DirectX::XMFLOAT3 Rotation)
{
	mTransform.Rotation = Rotation;
	mTransform.Rotation.x = fmodf(mTransform.Rotation.x, 360.0f);
	mTransform.Rotation.y = fmodf(mTransform.Rotation.y, 360.0f);	
	mTransform.Rotation.z = fmodf(mTransform.Rotation.z, 360.0f);
	
	OnSetTransform();
}

void WSceneComponent::SetLocalTransform(const FTransform& Transform)
{
	mTransform = Transform;

	OnSetTransform();
}

void WSceneComponent::SetLocalLocation(DirectX::XMFLOAT3 Location)
{
	mTransform.Translation = Location;

	OnSetTransform();
}

void WSceneComponent::SetLocalScale(DirectX::XMFLOAT3 Scale)
{
	mTransform.Scale = Scale;

	OnSetTransform();
}

void WSceneComponent::SetWorldTransform(FTransform Transform)
{
	if (auto Parent = mParent.lock())
	{
		XMMATRIX InvW = Parent->GetInverseWorldMatrix();
		XMMATRIX W = Transform.GetTransformMatrix();
		XMMATRIX M = W * InvW;
		XMVECTOR T;
		XMVECTOR QR;
		XMVECTOR S;
		XMMatrixDecompose(&S, &QR, &T, M);
		XMFLOAT4 Quat;
		XMStoreFloat4(&Quat, QR);
		XMStoreFloat3(&Transform.Translation, T);
		Transform.Rotation = FDXMath::QuaternionToEuler(Quat);
		XMStoreFloat3(&Transform.Scale, S);

		SetLocalTransform(Transform);
	}
	else
	{
		SetLocalTransform(Transform);
	}
}

void WSceneComponent::SetWorldLocation(XMFLOAT3 Location)
{
	FTransform Transform = GetWorldTransform();
	Transform.Translation = Location;
	SetWorldTransform(Transform);
}

void WSceneComponent::SetWorldRotation(XMFLOAT3 Rotation)
{
	FTransform Transform = GetWorldTransform();
	Transform.Rotation = Rotation;
	SetWorldTransform(Transform);
}

void WSceneComponent::SetWorldScale(XMFLOAT3 Scale)
{
	FTransform Transform = GetWorldTransform();
	Transform.Scale = Scale;
	SetWorldTransform(Transform);
}

void WSceneComponent::PropagateWorldFloat4Dirty(bool bForce)
{
	// 최적화: 내가 이미 Dirty라면 내 자식들도 이미 Dirty일 것이므로 중복 전파 중단
	if (mbWorldFloat4x4Dirty && !bForce)
	{
		return;
	}

	mbWorldFloat4x4Dirty = true;
	for (TWeakPtr<WSceneComponent> ChildWeak : mChilds)
	{
		// 핵심: 나(this)가 아니라 Child의 함수를 호출해야 함
		if (auto Child = ChildWeak.lock())
		{
			Child->PropagateWorldFloat4Dirty(bForce);
		}
	}
}

void WSceneComponent::Update()
{
	
}

void WSceneComponent::OnSetTransform()
{
	PropagateWorldFloat4Dirty();
}
