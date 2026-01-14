#include "Floor.h"
#include "Box.h"
#include "GameFramework/Object/World/World.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"
#include "GameFramework/Object/Component/SplineComponent.h"

AFloor::AFloor()
{
	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_LaminateFloorBrown));

	SplineComponent = CreateComponent<WSplineComponent>();
	SplineComponent->SetupAttachment(Component);
	SplineComponent->LoadSplineFromAsset(L"SDA_Up");
	SplineComponent->SetLocalLocation(XMFLOAT3(0.0f, 5.0f, 5.0f));

	mBody = CreateBoxBody({50, .5, 50}, EObjectType::EOT_Static);
	mBody->AddBody();
	mBody->SetActivate(false);
}

void AFloor::BeginPlay()
{
	Super::BeginPlay();

	mBody->SetPosition({ 0.0f, -3.0f, 0.0f });

	SplineFollowingActor = GetWorld()->SpawnActor<ABox>();

	GetWorld()->DrawDebugLine({ 0, 0, 2 }, { 2, 2, 2 }, { 1, 0, 0, 1 }, 5);
}

void AFloor::Tick_PrePhysics(float Delta)
{
	Super::Tick_PrePhysics(Delta);

	SetActorTransform(mBody->GetTransform());

	static std::array<std::wstring, 4> AssetNames = { L"SDA_Up" , L"SDA_Forward" , L"SDA_Right", L"SDA_ProjectilePath"};
	static int AssetSelector = 0;

	mCurrSplineDist += Delta;

	SplineFollowingActor->SetActorTransform(SplineComponent->GetWorldTransformAtDistanceAlongSpline(mCurrSplineDist));
	if (mCurrSplineDist > SplineComponent->GetSplineLength())
	{
		AssetSelector = (AssetSelector + 1) % AssetNames.size();
		SplineComponent->LoadSplineFromAsset(AssetNames[AssetSelector]);
		mCurrSplineDist = 0;
	}
}
