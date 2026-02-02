#include "Object.h"
#include "World/World.h"

using namespace BlueprintAsset;

WObject::WObject()
{
	mBlueprintPropertiesMap["this"] = this;
}

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

void WObject::RegisterWFunction(const std::string& Name, std::function<void(const TArray<BlueprintAsset::FProperty>&)> Func)
{
	assert(mWFunctions.count(Name) == 0 && "Already registered function name");
	mWFunctions[Name] = Func;
}

void WObject::LoadEvents(const TArray<BlueprintAsset::FEventNode>& Events)
{
	for (const auto& Event : Events)
	{
		mWEvents[Event.Name].LoadEvent(this, Event);
	}
}

void WObject::WEvent::LoadEvent(WObject* Context, const BlueprintAsset::FEventNode& Event)
{
	for (const auto& FuncNode : Event.Functions)
	{
		WObject* Target = Context->GetWPropertyPtr<WObject>(FuncNode.Target);
		assert(Target && "Target is not found");
		WFunction* Func = &Target->mWFunctions[FuncNode.Name];
		const WFunctionParamArray* Inputs = &FuncNode.Inputs;
		mFunctions.push_back([=]()
			{
				(*Func)(*Inputs);
			}
		);
	}
}

void WObject::WEvent::Dispatch() const
{
	for (const auto& Func : mFunctions)
	{
		Func();
	}
}