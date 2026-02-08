#include "Object.h"
#include "World/World.h"

WObject::WObject()
{
}

void WObject::Tick(float DeltaSecond)
{
}

void WObject::SetTickGroup(ETickGroup TickGroup, ETickPriority TickPriority)
{
	GetWorld()->DequeueTick(this);
	mTickGroup = TickGroup;
	mTickPriority = TickPriority;
	GetWorld()->EnqueueTick(this);
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

//void WObject::LoadWProperties(const TArray<FProperty>& Properties)
//{
//	for (const auto& Prop : Properties)
//	{
//		std::visit([&](auto& Arg)
//			{
//				using T = std::decay_t<decltype(Arg)>;
//				this->SetWProperty<T>(Prop.Name, Arg);
//			}, Prop.Value
//		);
//	}
//}
//
//void WObject::LoadWVariables(const TArray<FProperty>& Variables)
//{
//	for (const auto& v : Variables)
//	{
//		TSharedPtr<void> Value;
//
//		std::visit([&](auto&& Arg)
//			{
//				using T = std::decay_t<decltype(Arg)>;
//				Value = MakeShared<T>(Arg);
//			}, v.Value
//		);
//
//		mWVariables.push_back(Value);
//		RegisterWProperty(v.Name, Value.get());
//	}
//	
//	LoadWProperties(Variables);
//}

//void WObject::RegisterWFunction(const std::string& Name, WFunction Func)
//{
//	assert(mWFunctions.count(Name) == 0 && "Already registered function name");
//	RegisterWFunctionSafe(Name, Func);
//}
//
//void WObject::RegisterWFunctionSafe(const std::string& Name, WFunction Func)
//{
//	mWFunctions[Name] = Func;
//}

//void WObject::LoadEvents(WObject* Context, const TArray<BlueprintAsset::FEventNode>& Events)
//{
//	for (const auto& Event : Events)
//	{
//		mWEvents[Event.Name].LoadEvent(Context, Event);
//	}
//}
//
//void WObject::WEvent::LoadEvent(WObject* Context, const BlueprintAsset::FEventNode& Event)
//{
//	for (const auto& FuncNode : Event.Functions)
//	{
//		mFunctions.push_back([=]()
//			{
//				CallFunction(Context, FuncNode.get());
//			}
//		);
//	}
//}

//template<typename... Args>
//void DispathFunctionParamHelper(
//	BlueprintAsset::EPropertyType Type,
//	std::string Param,
//	TSharedPtr<void>& Value,
//	WObject* Target,
//	const std::variant<Args...>&
//)
//{
//	bool bSuccess = ( 
//		(Type == BlueprintAsset::PropertyTraits<Args>::Type ? (Value = MakeShared<Args>(Target->GetWProperty<Args>(Param)), true) : false)
//		|| ...);
//
//	assert(bSuccess && "Invalid type");
//};

//WObject::WFunctionReturn WObject::WEvent::CallFunction(WObject* Context, const BlueprintAsset::FFunctionNode* FuncNode)
//{
//	WObject* TargetObject = Context->GetWPropertyPtr<WObject>(FuncNode->Target);
//	if (!TargetObject)
//	{
//		TargetObject = Context->GetWPropertyPtr<WObject>(FuncNode->Target);
//	}
//	assert(TargetObject && "Invalid target");
//	WFunction& Function = TargetObject->mWFunctions[FuncNode->Call];
//	WFunctionParams Params;
//	for (const auto& StaticParam : FuncNode->StaticParameters)
//	{
//		TSharedPtr<WFunctionParameter> Param = MakeShared<WFunctionParameter>();
//		Param->Name = StaticParam.Name;
//
//		std::visit([&](auto&& Arg)
//			{
//				using T = std::decay_t<decltype(Arg)>;
//				Param->Value = MakeShared<T>(Arg);
//			}, StaticParam.Value
//		);
//		Params.push_back(std::move(Param));
//	}
//
//	for (const auto& PropParam : FuncNode->PropertyParameters)
//	{
//		TSharedPtr<WFunctionParameter> Param = MakeShared<WFunctionParameter>();
//
//		Param->Name = PropParam.Name;
//		WObject* Target = Context->GetWPropertyPtr<WObject>(PropParam.Target);
//		assert(Target != nullptr && "Invalide Target");
//
//		DispathFunctionParamHelper(
//			PropParam.Type,
//			PropParam.Value,
//			Param->Value,
//			Target,
//			BlueprintAsset::FProperty::FPropertyValue{}
//		);
//
//		//if (PropParam.Type == BlueprintAsset::EPropertyType::EPT_Boolean)
//		//{
//		//	Param->Value = MakeShared<bool>(Target->GetWProperty<bool>(PropParam.Value));
//		//}
//		//else if (PropParam.Type == BlueprintAsset::EPropertyType::EPT_Float)
//		//{
//		//	Param->Value = MakeShared<float>(Target->GetWProperty<float>(PropParam.Value));
//		//}
//		//else if (PropParam.Type == BlueprintAsset::EPropertyType::EPT_Float3)
//		//{
//		//	Param->Value = MakeShared<XMFLOAT3>(Target->GetWProperty<XMFLOAT3>(PropParam.Value));
//		//}
//		//else if (PropParam.Type == BlueprintAsset::EPropertyType::EPT_String)
//		//{
//		//	Param->Value = MakeShared<std::string>(Target->GetWProperty<std::string>(PropParam.Value));
//		//}
//		//else if (PropParam.Type == BlueprintAsset::EPropertyType::EPT_StringArray)
//		//{
//		//	Param->Value = MakeShared<TArray<std::string>>(Target->GetWProperty<TArray<std::string>>(PropParam.Value));
//		//}
//		//else
//		//{
//		//	assert(false && "Undefined property");
//		//}
//
//		Params.push_back(Param);
//	}
//
//	for (const auto& FuncParam : FuncNode->FunctionParameters)
//	{
//		TSharedPtr<WFunctionParameter> Param = MakeShared<WFunctionParameter>();
//		Param->Name = FuncParam->Name;
//		Param->Value = CallFunction(Context, FuncParam.get());
//		Params.push_back(Param);
//	}
//
//	return Function(Params);
//}