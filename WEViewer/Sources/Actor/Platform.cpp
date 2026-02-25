#include "Platform.h"
#include "Component/StaticMeshComponent.h"
#include "Component/BoxComponent.h"

APlatform::APlatform()
{
	mBoxComp = CreateComponent<WBoxComponent>();
	SetRootComponent(mBoxComp->GetWeakPtr<WSceneComponent>());
	mBoxComp->ActivatePhysicBody();
	mBoxComp->SetExtent(XMFLOAT3(0.5f, 0.05f, 0.5f));
	mBoxComp->SetMotionType(EMotionType::Static);
	mBoxComp->SetObjectChannel(EObjectChannel::EOC_PhysicsBody);

	mStaticMeshComp = CreateComponent<WStaticMeshComponent>();
	mStaticMeshComp->SetupAttachment(mBoxComp);
	mStaticMeshComp->SetStaticMesh("SM_WhiteBox");
	mStaticMeshComp->SetRelativeScale(XMFLOAT3(1, 0.1f, 1));
}

void APlatform::BeginPlay()
{
	Super::BeginPlay();

	mBoxComp->mOnHitDelegate.Bind(this, &APlatform::OnHit);
	mBoxComp->mOnExitHitDelegate.Bind(this, &APlatform::OnExitHit);
}

void APlatform::MovePlatform(const XMFLOAT3& NewLoc, bool bCarryPassengers)
{
	XMFLOAT3 CurrLoc = GetActorLocation();
	XMVECTOR vCurrLoc = XMLoadFloat3(&CurrLoc);
	XMVECTOR vNewLoc = XMLoadFloat3(&NewLoc);
	XMVECTOR vLocDelta = vNewLoc - vCurrLoc;

	for (TWeakPtr<AActor> ActorWeak : mContactedActor)
	{
		if (TSharedPtr<AActor> Actor = ActorWeak.lock())
		{
			XMFLOAT3 ActorLoc = Actor->GetActorLocation();
			if (ActorLoc.y <= CurrLoc.y)
			{
				continue;
			}

			XMVECTOR vActorLoc = XMLoadFloat3(&ActorLoc);
			vActorLoc += vLocDelta;
			XMStoreFloat3(&ActorLoc, vActorLoc);
			Actor->SetActorLocation(ActorLoc);
		}
	}

	SetActorLocation(NewLoc);
}

void APlatform::OnHit(WPhysicsComponent* OtherComp, XMFLOAT3 ImpactPoint)
{
	AActor* Actor = OtherComp->GetOwner<AActor>();

	int Count = (int)std::count_if(mContactedActor.begin(), mContactedActor.end(), [Actor](auto&& OtherWeak) -> bool
		{
			if (TSharedPtr<AActor> Other = OtherWeak.lock())
			{
				return Other.get() == Actor;
			}
			return false;
		});

	assert(Count < 2);
	
	if (Count == 0)
	{
		mContactedActor.push_back(Actor->GetWeakPtr<AActor>());
	}
}

void APlatform::OnExitHit(WPhysicsComponent* OtherComp)
{
	AActor* Actor = OtherComp->GetOwner<AActor>();

	auto Iter = std::remove_if(mContactedActor.begin(), mContactedActor.end(), [Actor](auto&& OtherActorWeak) {
		auto OtherActor = OtherActorWeak.lock();
		return !OtherActor || OtherActor.get() == Actor; // 만료된 포인터도 이때 같이 청소
		});
	mContactedActor.erase(Iter, mContactedActor.end()); // 실제로 벡터에서 제거
}
