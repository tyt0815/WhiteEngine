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

	TWeakPtr<AActor> GetOwner() const;

	bool HasTag(const std::string& Tag, bool bCheckOwner);

public:
	virtual void BeginComponent();

	AActor* mOwner;

public:

	friend class AActor;
	friend class WWorld;
};