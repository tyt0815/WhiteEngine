#include "WEProjectileAnim.h"
#include "GameFramework/GameAppImpl.h"
#include "Component/ObjectAnimComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/BoxComponent.h"
#include "Utility/Timer.h"
#include "GUI/GUICore.h"

CREATE_APPLICATION_BY_WORLD(WProjectileAnimWorld)

AProjectile::AProjectile()
{
	mBoxCollision = CreateComponent<WBoxComponent>();
	SetRootComponent(mBoxCollision);
	if (auto Box = mBoxCollision.lock())
	{
		Box->ActivatePhysicBody();
		Box->GenerateOverlapEvent();
		Box->SetExtent(XMFLOAT3(.5f, .5f, .5f));
		Box->SetMotionType(EMotionType::Kinematic);
		Box->SetObjectChannel(EObjectChannel::EOC_Moving);
	}

	mStaticMeshComp = CreateComponent<WStaticMeshComponent>();
	if (auto Comp = mStaticMeshComp.lock())
	{
		Comp->SetupAttachment(GetRootComponent());
		Comp->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_ScuffedGoldSphere));
	}
}

AProjectileAnimActor::AProjectileAnimActor()
{
	mObjectAnimComp = CreateComponent<WObjectAnimComponent>();
	if (auto ObjectAnimComp = mObjectAnimComp.lock())
	{
		ObjectAnimComp->SetupAttachment(GetRootComponent());

		ObjectAnimComp->LoadKeyframesFromOADAsset(L"OAD_Large");
	}
}

void AProjectileAnimActor::BeginPlay()
{
	Super::BeginPlay();

	mProj = GetWorld()->SpawnActor<AProjectile>();
}

void AProjectileAnimActor::Tick_PostPhysics(float Delta)
{
	Super::Tick_PostPhysics(Delta);

	mElapsedTime += Delta;

	if (auto ObjectAnimComp = mObjectAnimComp.lock())
	{
		mElapsedTime = fmodf(mElapsedTime, ObjectAnimComp->GetDuration());

		if (auto Proj = mProj.lock())
		{
			FTransform Transform = ObjectAnimComp->SampleAnimWorldTransformBySecond(mElapsedTime);
			Proj->SetActorTransform(Transform);
		}
	}
}

void AProjectileAnimActor::OnActivate()
{
	if (auto Proj = mProj.lock())
	{
		Proj->Activate();
	}
}

void AProjectileAnimActor::OnDeactivate()
{
	if (auto Proj = mProj.lock())
	{
		Proj->Deactivate();
	}
}

WProjectileAnimWorld::WProjectileAnimWorld()
{
	UTimer Timer;
	int ProjNum = 500;

	for (int i = 0; i < ProjNum; ++i)
	{
		if (auto Projectile = SpawnActor<AProjectileAnimActor>().lock())
		{
			float r = 360.0f / ProjNum * i;
			XMFLOAT3 Rot(r, r, r);
			Projectile->SetActorRotation(Rot);
			Projectile->Destroy();
		}
	}
	Timer.Tick();
	double SpawningTime = Timer.GetDeltaTime();
	

	GUI::FDrawCommand Command;
	Command.LifeSpan = 10;
	Command.DrawLambda = [=]()
	{
		ImGui::SetNextWindowPos(ImVec2(1000, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(300, 0));

		ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoMove;

		ImGui::SetNextWindowBgAlpha(1.0f);

		if (ImGui::Begin("Spawning", nullptr, WindowFlags))
		{

			ImGui::TextColored(ImVec4(0, 1, 1, 1), "Spawning Time");
			ImGui::Separator();

			ImGui::Text("%.8f", SpawningTime);
		}
		ImGui::End();
	};

	GUI::AddDrawCommand(Command);
}

void AProjSpawner::Tick_PrePhysics(float Delta)
{
	Super::Tick_PrePhysics(Delta);
}