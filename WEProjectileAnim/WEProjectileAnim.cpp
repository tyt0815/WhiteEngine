#include "WEProjectileAnim.h"
#include "GameFramework/GameAppImpl.h"
#include "Component/ObjectAnimComponent.h"
#include "Component/StaticMeshComponent.h"
#include "GUI/GUICore.h"
#include "Utility/Timer.h"

CREATE_APPLICATION_BY_WORLD(WProjectileAnimWorld)

AProjectile::AProjectile()
{
	mStaticMeshComp = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(mStaticMeshComp);
	if (auto Comp = mStaticMeshComp.lock())
	{
		Comp->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_ScuffedGoldSphere));
	}
}

AProjectileAnimActor::AProjectileAnimActor()
{
	mObjectAnimComp = CreateComponent<WObjectAnimComponent>();
	if (auto ObjectAnimComp = mObjectAnimComp.lock())
	{
		ObjectAnimComp->SetupAttachment(GetRootComponent());
		// ObjectAnimComp->LoadKeyframesFromXMLAsset(L"XDA_Projectile_Test");

		// 대용량 XML 테스트 블록
		{
			UTimer Timer;
			Timer.Reset();

			std::string XMLDir(SOLUTION_DIR);
			XMLDir += "Resources/XML";

			const char* MediumFileName = "Medium_v1.keyframemap.lz4";
			ObjectAnimComp->LoadKeyframesFromLZ4KeyframeMap(XMLDir + "/" + MediumFileName);
			Timer.Tick();
			float MediumTime = Timer.GetDeltaTime();

			const char* LargeFileName = "Large_v1.keyframemap.lz4";
			ObjectAnimComp->LoadKeyframesFromLZ4KeyframeMap(XMLDir + "/" + LargeFileName);
			Timer.Tick();
			float LargeTime = Timer.GetDeltaTime();

			GUI::FDrawCommand Command;
			Command.LifeSpan = 10;
			Command.DrawLambda = [=]()
			{
				ImGui::SetNextWindowPos(ImVec2(300, 0), ImGuiCond_Always);

				ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoDecoration |
					ImGuiWindowFlags_AlwaysAutoResize |
					ImGuiWindowFlags_NoSavedSettings |
					ImGuiWindowFlags_NoFocusOnAppearing |
					ImGuiWindowFlags_NoNav |
					ImGuiWindowFlags_NoMove;

				ImGui::SetNextWindowBgAlpha(1.0f);

				if (ImGui::Begin("XMLFileLoad", nullptr, WindowFlags))
				{
					ImGui::TextColored(ImVec4(1, 1, 0, 1), "XML file Load"); // 노란색 제목
					ImGui::Separator();

					ImGui::Text(
						"%s\nLoad Time: %f\n\n\n%s\nLoad Time: %f\n",
						MediumFileName, MediumTime,
						LargeFileName, LargeTime
					);
				}
				ImGui::End();
			};
			GUI::AddDrawCommand(Command);
		}
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
		mElapsedTime = fmodf(mElapsedTime, ObjectAnimComp->GetLastSecond());

		if (auto Proj = mProj.lock())
		{
			Proj->SetActorTransform(ObjectAnimComp->GetKeyframeWorldTransformBySecond(mElapsedTime));
		}		
	}
}

WProjectileAnimWorld::WProjectileAnimWorld()
{
	if (auto Projectile = SpawnActor<AProjectileAnimActor>().lock())
	{
		Projectile->SetActorLocation(XMFLOAT3(0, 0, 15));
	}
}