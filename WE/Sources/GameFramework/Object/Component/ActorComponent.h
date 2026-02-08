#pragma once

#include "Object.h"
#include "DirectX/DXMath.h"
#include "Utility/Memory.h"
#include "ComponentFactory.h"

class AActor;
class WWorld;

class WActorComponent : public WObject
{
	typedef WObject Super;
public:
	WActorComponent();

	virtual ~WActorComponent() noexcept = default;

	// TODO: 미구현
	void Destroy() override;

	// TODO: 미구현
	void Activate() override;

	// TODO: 미구현
	void Deactivate() override;

	TWeakPtr<AActor> GetOwner() const;

public:
	virtual void BeginComponent();

	AActor* mOwner;

public:

	friend class AActor;
	friend class WWorld;
};