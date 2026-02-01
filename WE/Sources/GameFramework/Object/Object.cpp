#include "Object.h"
#include "World/World.h"

using namespace BlueprintAsset;

void WObject::Tick(float DeltaSecond)
{
}

void WObject::SetTickGroup(ETickGroup TickGroup, ETickPriority TickPriority)
{
	mTickGroup = TickGroup;
	mTickPriority = TickPriority;
}

void WObject::OnDestroy()
{
	Deactivate();
}

void WObject::OnActivate()
{
	GetWorld()->EnqueueTick(this);
}

void WObject::OnDeactivate()
{
	GetWorld()->DequeueTick(this);
}

void WObject::LoadWProperties(const TArray<FProperty>& Properties)
{
	TArray<const FProperty*> RawProperties;
	for (const auto& Prop : Properties)
	{
		if (Prop.Type == EPropertyType::EPT_Float)
		{
			SetWProperty<float>(Prop.Name, std::get<float>(Prop.Value));
		}
		else if (Prop.Type == EPropertyType::EPT_Boolean)
		{
			SetWProperty<bool>(Prop.Name, std::get<bool>(Prop.Value));
		}
		else
		{
			RawProperties.push_back(&Prop);
		}
	}
}

void WObject::LoadWInitializers(const TArray<BlueprintAsset::FInitializer>& Initializers)
{
}
