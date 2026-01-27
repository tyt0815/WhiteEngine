#include "Object.h"
#include "World/World.h"

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
