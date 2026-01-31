#pragma once
#include <d3d12.h>
#include <vector>
#include "DirectX/DXMath.h"
#include "Utility/Class.h"
#include "MeshGeometry.h"
#include "Material.h"

class FMeshGeometry;
class FMaterial;

struct FStaticMesh
{
	FMeshGeometry* Geometry = nullptr;
	FMaterial* Material = nullptr;
};

class FStaticMeshManager
{
	SINGLETON(FStaticMeshManager);
public:

private:
	void BuildStaticMeshs();
	void BuildStaticMesh(const std::string& Name, std::string MeshName, EMaterialType MaterialType);
	std::unordered_map<std::string, FStaticMesh> mStaticMeshs;

private:
	__forceinline FStaticMesh GetStaticMesh_Internal(const std::string& Name)
	{
		return mStaticMeshs[Name];
	}
public:

	static __forceinline FStaticMesh GetStaticMesh(const std::string& Name)
	{
		return GetInstance()->GetStaticMesh_Internal(Name);
	}
};

inline FStaticMeshManager* GetStaticMeshManager()
{
	return FStaticMeshManager::GetInstance();
}