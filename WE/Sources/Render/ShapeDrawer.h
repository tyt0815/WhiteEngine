#pragma once

#include <d3d12.h>
#include <vector>
#include "Utility/String.h"
#include "Utility/Class.h"

class FMeshGeometry;

class FShapeDrawer
{
	SINGLETON(FShapeDrawer);
public:
	std::vector<D3D12_INPUT_ELEMENT_DESC> GetDrawingSphereInputLayouts();
	void DrawSphere(ID3D12GraphicsCommandList* CommandList);

private:
	void BuildBuffers(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	void BuildSphereBuffer(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	
	FMeshGeometry* mSphere = nullptr;
	std::string mSphereName = "Sphere_Simple";

};

inline FShapeDrawer* GetShapeDrawer()
{
	return FShapeDrawer::GetInstance();
}