#include "Object.h"
#include "World/World.h"

using namespace BlueprintAsset;

WObject::WObject()
{
	RegisterWProperty("this", this);

	REGISTER_GET_SELF(WObject);

	mTickEvent = RegisterEvent("Tick");
}

void WObject::Tick(float DeltaSecond)
{
	mTickEvent->Dispatch();
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
	for (const auto& Prop : Properties)
	{
		if (Prop.Type == EPropertyType::EPT_Float)
		{
			SetWProperty<float>(Prop.Name, std::get<float>(Prop.Value));
		}
		else if (Prop.Type == EPropertyType::EPT_Float3)
		{
			SetWProperty<XMFLOAT3>(Prop.Name, std::get<XMFLOAT3>(Prop.Value));
		}
		else if (Prop.Type == EPropertyType::EPT_Boolean)
		{
			SetWProperty<bool>(Prop.Name, std::get<bool>(Prop.Value));
		}
		else if (Prop.Type == EPropertyType::EPT_String)
		{
			SetWProperty<std::string>(Prop.Name, std::get<std::string>(Prop.Value));
		}
		else if (Prop.Type == EPropertyType::EPT_StringArray)
		{
			SetWProperty<std::vector<std::string>>(Prop.Name, std::get<std::vector<std::string>> (Prop.Value));
		}
		else
		{
			assert(false && "Undefined Property");
		}
	}
}

void WObject::LoadWVariables(const TArray<FProperty>& Variables)
{
	for (const auto& v : Variables)
	{
		TSharedPtr<void> Value;
		if (v.Type == EPropertyType::EPT_Float)
		{
			Value = MakeShared<float>(std::get<float>(v.Value));
		}
		else if (v.Type == EPropertyType::EPT_Float3)
		{
			Value = MakeShared<XMFLOAT3>(std::get<XMFLOAT3>(v.Value));
		}
		else if (v.Type == EPropertyType::EPT_Boolean)
		{
			Value = MakeShared<bool>(std::get<bool>(v.Value));
		}
		else if (v.Type == EPropertyType::EPT_String)
		{
			Value = MakeShared<std::string>(std::get<std::string>(v.Value));
		}
		else if (v.Type == EPropertyType::EPT_StringArray)
		{
			Value = MakeShared<std::vector<std::string>>(std::get<std::vector<std::string>>(v.Value));
		}
		else
		{
			assert(false && "Undefined Property");
		}

		mWVariables.push_back(Value);
		RegisterWProperty(v.Name, Value.get());
	}

	LoadWProperties(Variables);
}

void WObject::RegisterWFunction(const std::string& Name, WFunction Func)
{
	assert(mWFunctions.count(Name) == 0 && "Already registered function name");
	RegisterWFunctionSafe(Name, Func);
}

void WObject::RegisterWFunctionSafe(const std::string& Name, WFunction Func)
{
	mWFunctions[Name] = Func;
}

void WObject::LoadEvents(WObject* Context, const TArray<BlueprintAsset::FEventNode>& Events)
{
	for (const auto& Event : Events)
	{
		mWEvents[Event.Name].LoadEvent(Context, Event);
	}
}

void WObject::WEvent::LoadEvent(WObject* Context, const BlueprintAsset::FEventNode& Event)
{
	for (const auto& FuncNode : Event.Functions)
	{
		mFunctions.push_back([=]()
			{
				CallFunction(Context, FuncNode.get());
			}
		);
	}
}

WObject::WFunctionReturn WObject::WEvent::CallFunction(WObject* Context, const BlueprintAsset::FFunctionNode* FuncNode)
{
	WFunction& Function = Context->GetWPropertyPtr<WObject>(FuncNode->Target)->mWFunctions[FuncNode->Call];
	WFunctionParams Params;
	for (const auto& StaticParam : FuncNode->StaticParameters)
	{
		TSharedPtr<WFunctionParameter> Param = MakeShared<WFunctionParameter>();
		Param->Name = StaticParam.Name;
		if (StaticParam.Type == BlueprintAsset::EPropertyType::EPT_Boolean)
		{
			Param->Value = MakeShared<bool>(std::get<bool>(StaticParam.Value));
		}
		else if (StaticParam.Type == BlueprintAsset::EPropertyType::EPT_Float)
		{
			Param->Value = MakeShared<float>(std::get<float>(StaticParam.Value));
		}
		else if (StaticParam.Type == BlueprintAsset::EPropertyType::EPT_Float3)
		{
			Param->Value = MakeShared<XMFLOAT3>(std::get<XMFLOAT3>(StaticParam.Value));
		}
		else if (StaticParam.Type == BlueprintAsset::EPropertyType::EPT_String)
		{
			Param->Value = MakeShared<std::string>(std::get<std::string>(StaticParam.Value));
		}
		else if (StaticParam.Type == BlueprintAsset::EPropertyType::EPT_StringArray)
		{
			Param->Value = MakeShared<std::vector<std::string>>(std::get<std::vector<std::string>>(StaticParam.Value));
		}
		else
		{
			assert(false && "Undefined property");
		}
		Params.push_back(std::move(Param));
	}

	for (const auto& PropParam : FuncNode->PropertyParameters)
	{
		TSharedPtr<WFunctionParameter> Param = MakeShared<WFunctionParameter>();

		Param->Name = PropParam.Name;
		WObject* Target = Context->GetWPropertyPtr<WObject>(PropParam.Target);
		assert(Target != nullptr && "Invalide Target");
		if (PropParam.Type == BlueprintAsset::EPropertyType::EPT_Boolean)
		{
			Param->Value = MakeShared<bool>(Target->GetWProperty<bool>(PropParam.Value));
		}
		else if (PropParam.Type == BlueprintAsset::EPropertyType::EPT_Float)
		{
			Param->Value = MakeShared<float>(Target->GetWProperty<float>(PropParam.Value));
		}
		else if (PropParam.Type == BlueprintAsset::EPropertyType::EPT_Float3)
		{
			Param->Value = MakeShared<XMFLOAT3>(Target->GetWProperty<XMFLOAT3>(PropParam.Value));
		}
		else if (PropParam.Type == BlueprintAsset::EPropertyType::EPT_String)
		{
			Param->Value = MakeShared<std::string>(Target->GetWProperty<std::string>(PropParam.Value));
		}
		else if (PropParam.Type == BlueprintAsset::EPropertyType::EPT_StringArray)
		{
			Param->Value = MakeShared<TArray<std::string>>(Target->GetWProperty<TArray<std::string>>(PropParam.Value));
		}
		else
		{
			assert(false && "Undefined property");
		}

		Params.push_back(Param);
	}

	for (const auto& FuncParam : FuncNode->FunctionParameters)
	{
		TSharedPtr<WFunctionParameter> Param = MakeShared<WFunctionParameter>();
		Param->Name = FuncParam->Name;
		Param->Value = CallFunction(Context, FuncParam.get());
		Params.push_back(Param);
	}

	return Function(Params);
}

void WObject::WEvent::Dispatch() const
{
	for (const auto& Func : mFunctions)
	{
		Func();
	}
}
