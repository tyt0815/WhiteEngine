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
	mbActivate = true;
	OnActivate();
}

void WObject::Deactivate()
{
	mbActivate = false;
	OnDeactivate();
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

void WObject::AddTags(TArray<std::string>& Tags)
{
	mTags.insert(Tags.begin(), Tags.end());
}
