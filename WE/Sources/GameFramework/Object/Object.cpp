#include "Object.h"
#include "World/World.h"

void WObject::Tick(float DeltaSecond)
{
}

void WObject::SetTickGroup(ETickGroup::ETickGroup TickGroup, ETickPriority::ETickPriority TickPriority)
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
