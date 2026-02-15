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

void WObject::Activate()
{
	if (!mbActivate)
	{
		mbActivate = true;
		OnActivate();
	}
}

void WObject::Deactivate()
{
	if (mbActivate)
	{
		mbActivate = false;
		OnDeactivate();
	}
}

void WObject::OnDestroy()
{
	Deactivate();
}

void WObject::OnActivate()
{
	GetWorld()->EnqueueTick(this);
	mOnActivate.Broadcast();
}

void WObject::OnDeactivate()
{
	GetWorld()->DequeueTick(this);
	mOnDeactivate.Broadcast();
}

void WObject::RegisterWProperty(const std::string& Name, WEvalValue Value)
{
	std::visit([=](auto&& v) 
		{
			RegisterWProperty(Name, v);
		}, Value
	);
}

void WObject::RegisterWFunction(const std::string& Name, std::function<WEvalValue()> Lambda)
{
	auto Iter = mWFunctionsMap.find("Name");
	if (Iter != mWFunctionsMap.end())
	{
		std::cout << "Already registered WFunction: " + Name << std::endl;
		return;
	}

	mWFunctionsMap[Name] = Lambda;
}

WSourceRef WObject::GetWPropertyPtr(const std::string& Name)
{
	auto Iter = mWPropertiesMap.find(Name);
	if (Iter == mWPropertiesMap.end())
	{
		return WSourceRef();
	}
	return Iter->second;
}

void WObject::SetWPropertyValue(const std::string& Name, WEvalValue Value)
{
	auto it = mWPropertiesMap.find(Name);
	if (it == mWPropertiesMap.end()) return;

	std::visit([](auto&& TargetPtr, auto&& SourceValue)
		{
			using TPtr = std::decay_t<decltype(TargetPtr)>;
			using TVal = std::decay_t<decltype(SourceValue)>;

			using ValueType = std::remove_pointer_t<TPtr>;

			if constexpr (std::is_same<ValueType, TVal>::value)
			{
				if (TargetPtr) 
				{
					*TargetPtr = SourceValue;
				}
			}
			else
			{
				std::cout << "Invalid WProperty type" << std::endl;
			}
		}, it->second, Value);
}

WEvalValue WObject::ExecuteWFunction(const std::string& Name) const
{
	auto Iter = mWFunctionsMap.find(Name);
	if (Iter == mWFunctionsMap.end())
	{
		std::cout << "Unregistered WFunction: " + Name << std::endl;
		return WEvalValue();
	}

	return Iter->second();
}

void WObject::AddTags(TArray<std::string>& Tags)
{
	mTags.insert(Tags.begin(), Tags.end());
}