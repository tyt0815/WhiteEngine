#include "WEProjectileAnim.h"
#include "GameFramework/GameAppImpl.h"
#include "Component/ObjectAnimComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Utility/Timer.h"
#include "GUI/GUICore.h"

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

		// 대용량 XML 테스트 블록
		{
			UTimer Timer;
			Timer.Reset();

			std::string XMLDir(SOLUTION_DIR);
			XMLDir += "Resources/XML";

			//const char* MediumFileName = "Medium_v1.keyframemap.lz4";
			//ObjectAnimComp->LoadKeyframesFromLZ4KeyframeMap(XMLDir + "/" + MediumFileName);
			//Timer.Tick();
			//float MediumTime = Timer.GetDeltaTime();

			const char* LargeFileName = "Large_v1.keyframemap.lz4";
			ObjectAnimComp->LoadKeyframesFromLZ4KeyframeMap(XMLDir + "/" + LargeFileName);
			Timer.Tick();
			float LargeTime = Timer.GetDeltaTime();

			static bool bFirstTime = true;
			if (bFirstTime)
			{
				bFirstTime = false;
				GUI::FDrawCommand Command;
				Command.LifeSpan = 10;
				Command.DrawLambda = [=]()
				{
					ImGui::SetNextWindowPos(ImVec2(1000, 0), ImGuiCond_Always);

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

						ImGui::Text("%s\nLoad Time: %f",LargeFileName, LargeTime);
					}
					ImGui::End();
				};
				GUI::AddDrawCommand(Command);
			}
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

	UTimer Timer;
	float SampleAnimTime = 0;
	if (auto ObjectAnimComp = mObjectAnimComp.lock())
	{
		mElapsedTime = fmodf(mElapsedTime, ObjectAnimComp->GetLastSecond());

		if (auto Proj = mProj.lock())
		{
			
			Timer.Reset();
			FTransform Transform = ObjectAnimComp->SampleAnimWorldTransformBySecond(mElapsedTime);
			Timer.Tick();
			SampleAnimTime = Timer.GetDeltaTime();

			Proj->SetActorTransform(Transform);
		}
	}

	static TWeakPtr<AProjectileAnimActor> FirstActor = GetWeakPtr<AProjectileAnimActor>();
	static float One_SampleAnimTimeAvg = 0;
	static float Whole_SampleAnimTimeAvg = 0;
	static float Alpha = 0.01f;

	if (FirstActor.expired())
	{
		FirstActor = GetWeakPtr<AProjectileAnimActor>();
	}

	if (Whole_SampleAnimTimeAvg == 0)
	{
		Whole_SampleAnimTimeAvg = SampleAnimTime;
	}
	else
	{
		Whole_SampleAnimTimeAvg = (1 - Alpha) * Whole_SampleAnimTimeAvg + Alpha * SampleAnimTime;
	}
	if (FirstActor.lock().get() == this)
	{
		if (One_SampleAnimTimeAvg == 0)
		{
			One_SampleAnimTimeAvg = SampleAnimTime;
		}
		else
		{
			One_SampleAnimTimeAvg = (1 - Alpha) * One_SampleAnimTimeAvg + Alpha * SampleAnimTime;
		}

		// 프로파일링용 GUI
		GUI::FDrawCommand Command;
		Command.LifeSpan = 0;
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

			if (ImGui::Begin("AProjectileAnimActor::Tick_PostPhysics", nullptr, WindowFlags))
			{
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "ProjectileAnimActor::Tick_PostPhysics"); // 노란색 제목
				ImGui::Separator();

				ImGui::Text("KeyframeSearchTime: %f", SampleAnimTime);
				ImGui::Text("One_SampleAnimTimeAvg: %f", One_SampleAnimTimeAvg);
				ImGui::Text("Whole_SampleAnimTimeAvg: %f", Whole_SampleAnimTimeAvg);
			}
			ImGui::End();
		};
		GUI::AddDrawCommand(Command);
	}
}

WProjectileAnimWorld::WProjectileAnimWorld()
{
	int ProjNum = 100;
	for (int i = 0; i < ProjNum; ++i)
	{
		if (auto Projectile = SpawnActor<AProjectileAnimActor>().lock())
		{
			XMFLOAT3 Rot(0, 0, 3.6f * i);
			Projectile->SetActorRotation(Rot);
		}
	}

}