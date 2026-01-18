#include "RenderItemProxy.h"

void FRenderItemProxy::Cleanup(float Delta)
{
	mMeshCBProxies.clear();
	mSubmeshCBProxies.clear();
	mStaticMeshProxies.clear();
	mDirectionalLightProxies.clear();
	
	for (int i = 0; i < mDebugLine3DProxies.size(); ++i)
	{
		mDebugLine3DProxies[i].LifeSpan -= Delta;
		if (mDebugLine3DProxies[i].LifeSpan < 0)
		{
			mDebugLine3DProxies[i--] = mDebugLine3DProxies.back();
			mDebugLine3DProxies.pop_back();
		}
	}
}

size_t FRenderItemProxy::AllocateMeshCbProxy()
{
	mMeshCBProxies.push_back({});
	return mMeshCBProxies.size() - 1;
}

size_t FRenderItemProxy::AllocateSubmeshCbProxy()
{
	mSubmeshCBProxies.push_back({});
	return mSubmeshCBProxies.size() - 1;
}

size_t FRenderItemProxy::AllocateStaticMeshCbProxy()
{
	mStaticMeshProxies.push_back({});
	return mStaticMeshProxies.size() - 1;
}

size_t FRenderItemProxy::AllocateDirectionalLightCbProxy()
{
	mDirectionalLightProxies.push_back({});
	return mDirectionalLightProxies.size() - 1;
}
