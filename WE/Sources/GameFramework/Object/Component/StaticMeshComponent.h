#pragma once
#include "PrimitiveComponent.h"
#include <array>
#include "Render/StaticMesh.h"
#include "Render/Material.h"

class WStaticMeshComponent : public WPrimitiveComponent
{
	typedef WPrimitiveComponent Super;
public:
	WStaticMeshComponent();

protected:
	virtual void UpdateConstantBufferIndex() override;

	virtual void UpdateProxies() override;

	virtual void CreatePhysicsBody() override {};		// TODO

public:
	void SetStaticMesh(const FStaticMesh& StaticMesh);

private:
	FStaticMesh mStaticMesh;
	std::vector<size_t> mStaticMeshProxyIndecies;
	std::vector<size_t> mSubmeshCBIndices;
	bool mbCastShadow = true;

public:
	inline void SetCastShadow(bool bCast)
	{
		mbCastShadow = bCast;
	}
};