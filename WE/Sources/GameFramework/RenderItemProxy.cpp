#include "RenderItemProxy.h"

void FRenderItemProxy::Cleanup()
{
	mMeshCBProxies.clear();
	mSubmeshCBProxies.clear();
	mStaticMeshProxies.clear();
	mDirectionalLightProxies.clear();
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
