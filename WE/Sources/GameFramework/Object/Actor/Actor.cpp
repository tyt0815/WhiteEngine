#include "Actor.h"
#include "Component/SceneComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/SplineComponent.h"
#include "Component/ObjectAnimComponent.h"
#include "World/World.h"
#include "Asset/BlueprintAsset.h"
#include "Parser.h"

unsigned int g_ActorCounter = 0;

AActor::AActor():
	mActorCounter(++g_ActorCounter)
{
	TWeakPtr<WSceneComponent> DummyRoot = CreateComponent<WSceneComponent>()->GetWeakPtr<WSceneComponent>();

	SetRootComponent(DummyRoot);

	RegisterWProperty<float>("DeltaTime", &mDeltaTime);
}

void AActor::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	mDeltaTime = DeltaSecond;

	mElapsedTime += DeltaSecond;

	// mSplineFollowInfos는 std::vector 또는 std::list라고 가정합니다.
	for (auto it = mSplineFollowInfos.begin(); it != mSplineFollowInfos.end(); )
	{
		FSplineFollowInfo& Info = *it;

		// 1. 유효성 검사
		if (!Info.Target || !Info.Spline)
		{
			it = mSplineFollowInfos.erase(it);
			continue;
		}

		// 2. 시간 누적 및 비율 계산
		Info.ElapsedTime += DeltaSecond;
		float Alpha = Info.ElapsedTime / Info.Duration;

		// 3. 종료 및 루프 판정
		bool bFinished = false;
		if (Alpha >= 1.0f)
		{
			if (Info.bLoop)
			{
				// 루프 시 초과된 시간을 나머지 연산으로 넘겨주어 프레임 끊김 방지
				Info.ElapsedTime = fmodf(Info.ElapsedTime, Info.Duration);
				Alpha = Info.ElapsedTime / Info.Duration;
			}
			else
			{
				Alpha = 1.0f;
				bFinished = true;
			}
		}

		// 4. 스플라인 거리 기반 샘플링
		// 비율(Alpha)을 전체 길이(GetSplineLength)에 곱해 현재 가야 할 거리를 구합니다.
		float TotalLength = Info.Spline->GetSplineLength();
		float TargetDistance = TotalLength * Alpha;

		// 5. 트랜스폼 업데이트
		// GetWorldTransformAtDistanceAlongSpline을 사용하여 월드 좌표계 이동 지원
		FTransform NewTransform = Info.Spline->GetWorldTransformAtDistanceAlongSpline(TargetDistance);

		// 위치 적용
		Info.Target->SetWorldLocation(NewTransform.Translation);

		// 옵션에 따른 회전 적용
		if (Info.bUseRotation)
		{
			// FTransform 내에 XMFLOAT3 타입의 Rotation(Euler)이 있다고 가정하거나, 
			// 필요 시 Quaternion을 사용하여 회전을 직접 설정합니다.
			Info.Target->SetWorldRotation(NewTransform.Rotation);
		}

		// 6. 반복자 관리
		if (bFinished)
		{
			it = mSplineFollowInfos.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void AActor::Destroy()
{
	if (!IsPendingKill())
	{
		GetWorld()->DestroyActor(GetWeakPtr<AActor>().lock());
		//mOnDestroyEvent.Dispatch();
	}
}

void AActor::OnDestroy()
{
	Super::OnDestroy();

	for (auto Comp : mAllComponents)
	{
		Comp->OnDestroy();
	}
}

void AActor::OnActivate()
{
	Super::OnActivate();
	GetWorld()->ActivateActor(this);
	for (auto& Comp : mAllComponents)
	{
		if (Comp->IsActivate())
		{
			Comp->OnActivate();
		}
	}
}

void AActor::OnDeactivate()
{
	for (auto& Comp : mAllComponents)
	{
		Comp->OnDeactivate();
	}
	GetWorld()->DeactivateActor(this);
	Super::OnDeactivate();
}

void AActor::BeginPlay()
{
	if (IsActivate())
	{
		OnActivate();
		BeginComponents();
	}
}

XMFLOAT3 AActor::GetForwardVector() const
{
	XMMATRIX RotationMatrix = GetActorTransform().GetRotationMatrix();
	XMVECTOR L = XMVector3Transform({ 0.0f, 0.0f, 1.0f }, RotationMatrix);
	XMFLOAT3 Foward;
	XMStoreFloat3(&Foward, XMVector3Normalize(L));
	return Foward;
}

XMFLOAT3 AActor::GetRightVector() const
{
	XMMATRIX RotationMatrix = GetActorTransform().GetRotationMatrix();
	XMVECTOR R = XMVector3Transform({ 1.0f, 0.0f, 0.0f }, RotationMatrix);
	XMFLOAT3 Right;
	XMStoreFloat3(&Right, R);
	return Right;
}

XMFLOAT3 AActor::GetUpVector() const
{
	XMMATRIX RotationMatrix = GetActorTransform().GetRotationMatrix();
	XMVECTOR U = XMVector3Transform({ 0.0f, 1.0f, 0.0f }, RotationMatrix);
	XMFLOAT3 Up;
	XMStoreFloat3(&Up, U);
	return Up;
}

XMFLOAT4 AActor::GetActorQuaternion()
{
	return mRootComponent.lock()->GetWorldQuatRotation();
}

void AActor::SetRootComponent(TWeakPtr<WSceneComponent> Component)
{
	TSharedPtr<WSceneComponent> OldRoot = mRootComponent.lock();
	TSharedPtr<WSceneComponent> NewRoot = Component.lock();
	if (OldRoot && NewRoot)
	{
		if (OldRoot.get() == NewRoot.get())
		{
			return;
		}
		else
		{
			OldRoot->SetupAttachment(NewRoot.get());
		}
	}

	if (OldRoot)
	{
		OldRoot->PropagateWorldFloat4Dirty(true);
	}
	if (NewRoot)
	{
		NewRoot->PropagateWorldFloat4Dirty(true);
	}

	mRootComponent = Component;
}

void AActor::SetActorTransform(FTransform Transform)
{
	if (auto Root = mRootComponent.lock())
	{
		Root->SetRelativeTransform(Transform);
	}
}

void AActor::OnCreateComponent(WActorComponent* Comp)
{
	Comp->SetOwner(this);
	if (WSceneComponent* SceneComp = dynamic_cast<WSceneComponent*>(Comp))
	{
		mAllSceneComponent.emplace_back(SceneComp->GetWeakPtr<WSceneComponent>());
		if (WPhysicsComponent* PhysicsComp = dynamic_cast<WPhysicsComponent*>(SceneComp))
		{
			mAllPhysicsComponents.emplace_back(PhysicsComp->GetWeakPtr<WPhysicsComponent>());
		}
	}
	else
	{
		mAllNoneSceneComponent.emplace_back(Comp->GetWeakPtr<WActorComponent>());
	}
}

void AActor::UpdateRecursive()
{
	if (auto Root = mRootComponent.lock())
	{
		Root->UpdateRecursive();
	}
}

void AActor::BeginComponents()
{
	for (int i = 0; i < mAllComponents.size(); ++i)
	{
		mAllComponents[i]->BeginComponent();
	}
}

void AActor::LoadBlueprint(const FBlueprintAsset* Asset)
{
	if (!Asset) return;

	for (const auto& Setup : Asset->mComponentSetups)
	{
		// Registry에 등록된 CreateFunc를 실행하여 컴포넌트 인스턴스 생성
		WActorComponent* NewComp = Setup.CreateFunc(this);
		if (!NewComp) continue;

		// 컴포넌트 리스트에 등록 (GetWComponent 등으로 찾기 위해)
		RegisterWObject(Setup.Name, NewComp);

		const std::string& Prefix = Setup.Name + ".";

		// 계층 구조 처리 (SceneComponent인 경우)
		WSceneComponent* SceneComp = dynamic_cast<WSceneComponent*>(NewComp);
		if (SceneComp)
		{
			if (!Setup.ParentName.empty())
			{
				
				if (WSceneComponent* Parent = GetWObject<WSceneComponent>(Setup.ParentName))
				{
					SceneComp->SetupAttachment(Parent);
				}
				else
				{
					std::cout << Setup.ParentName + " is not Scene component" << std::endl;
				}
			}
			else if(Setup.Name == "Root")
			{
				SetRootComponent(SceneComp->GetWeakPtr<WSceneComponent>());
			}
			else
			{
				SceneComp->SetupAttachment(GetRootComponent());
			}
		}
	}
}