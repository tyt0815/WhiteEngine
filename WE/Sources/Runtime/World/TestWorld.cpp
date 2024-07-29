#include "TestWorld.h"
#include "Render/MeshGeometry.h"

WTestWorld::WTestWorld():
	Super()
{
}

void WTestWorld::BuildWorldActors()
{
	FMeshGeometry::MeshGeometryMap& Geometries = FMeshGeometry::MeshGeometries;
	FMaterial::MaterialMap& Materials = FMaterial::Materials;

	//vector<unique_ptr<AActor>> BuildTargetActor;

	//auto boxRitem = std::make_unique<AActor>();
	//boxRitem->SetScale(XMFLOAT3(2.0f, 2.0f, 2.0f));
	//boxRitem->SetTranslation(XMFLOAT3(0.0f, 0.5f, 0.0f));
	//XMStoreFloat4x4(&boxRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	//boxRitem->ObjectConstantBufferIndex = 0;
	//boxRitem->Material = Materials["Stone0"].get();
	//boxRitem->Geometry = Geometries["Shape"].get();
	//boxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//boxRitem->IndexCount = boxRitem->Geometry->DrawArgs["Box"].IndexCount;
	//boxRitem->StartIndexLocation = boxRitem->Geometry->DrawArgs["Box"].StartIndexLocation;
	//boxRitem->BaseVertexLocation = boxRitem->Geometry->DrawArgs["Box"].BaseVertexLocation;
	//BuildTargetActor.push_back(std::move(boxRitem));

	//auto gridRitem = std::make_unique<AActor>();
	//XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(8.0f, 8.0f, 1.0f));
	//gridRitem->ObjectConstantBufferIndex = 1;
	//gridRitem->Material = Materials["Tile0"].get();
	//gridRitem->Geometry = Geometries["Shape"].get();
	//gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//gridRitem->IndexCount = gridRitem->Geometry->DrawArgs["Grid"].IndexCount;
	//gridRitem->StartIndexLocation = gridRitem->Geometry->DrawArgs["Grid"].StartIndexLocation;
	//gridRitem->BaseVertexLocation = gridRitem->Geometry->DrawArgs["Grid"].BaseVertexLocation;
	//BuildTargetActor.push_back(std::move(gridRitem));

	//auto skullRitem = std::make_unique<AActor>();
	//skullRitem->SetScale(XMFLOAT3(0.5f, 0.5f, 0.5f));
	//skullRitem->SetTranslation(XMFLOAT3(0.0f, 1.0f, 0.0f));
	//skullRitem->TexTransform = UDXMath::Identity4x4();
	//skullRitem->ObjectConstantBufferIndex = 2;
	//skullRitem->Material = Materials["Skull"].get();
	//skullRitem->Geometry = Geometries["Skull"].get();
	//skullRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//skullRitem->IndexCount = skullRitem->Geometry->DrawArgs["Skull"].IndexCount;
	//skullRitem->StartIndexLocation = skullRitem->Geometry->DrawArgs["Skull"].StartIndexLocation;
	//skullRitem->BaseVertexLocation = skullRitem->Geometry->DrawArgs["Skull"].BaseVertexLocation;
	//BuildTargetActor.push_back(std::move(skullRitem));

	//XMMATRIX BrickTexTransform = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	//UINT objCBIndex = (UINT)BuildTargetActor.size();
	//for (int i = 0; i < 5; ++i)
	//{
	//	auto leftCylRitem = std::make_unique<AActor>();
	//	auto rightCylRitem = std::make_unique<AActor>();
	//	auto leftSphereRitem = std::make_unique<AActor>();
	//	auto rightSphereRitem = std::make_unique<AActor>();

	//	XMMATRIX leftCylWorld = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f);
	//	XMMATRIX rightCylWorld = XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f);

	//	XMMATRIX leftSphereWorld = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f);
	//	XMMATRIX rightSphereWorld = XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f);

	//	//XMStoreFloat4x4(&leftCylRitem->World, rightCylWorld);
	//	leftCylRitem->SetTranslation(XMFLOAT3(+5.0f, 1.5f, -10.0f + i * 5.0f));
	//	XMStoreFloat4x4(&leftCylRitem->TexTransform, BrickTexTransform);
	//	leftCylRitem->ObjectConstantBufferIndex = objCBIndex++;
	//	leftCylRitem->Geometry = Geometries["Shape"].get();
	//	leftCylRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//	leftCylRitem->IndexCount = leftCylRitem->Geometry->DrawArgs["Cylinder"].IndexCount;
	//	leftCylRitem->StartIndexLocation = leftCylRitem->Geometry->DrawArgs["Cylinder"].StartIndexLocation;
	//	leftCylRitem->BaseVertexLocation = leftCylRitem->Geometry->DrawArgs["Cylinder"].BaseVertexLocation;

	//	//XMStoreFloat4x4(&rightCylRitem->World, leftCylWorld);
	//	rightCylRitem->SetTranslation(XMFLOAT3(-5.0f, 1.5f, -10.0f + i * 5.0f));
	//	XMStoreFloat4x4(&rightCylRitem->TexTransform, BrickTexTransform);
	//	rightCylRitem->ObjectConstantBufferIndex = objCBIndex++;
	//	rightCylRitem->Geometry = Geometries["Shape"].get();
	//	rightCylRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//	rightCylRitem->IndexCount = rightCylRitem->Geometry->DrawArgs["Cylinder"].IndexCount;
	//	rightCylRitem->StartIndexLocation = rightCylRitem->Geometry->DrawArgs["Cylinder"].StartIndexLocation;
	//	rightCylRitem->BaseVertexLocation = rightCylRitem->Geometry->DrawArgs["Cylinder"].BaseVertexLocation;

	//	//XMStoreFloat4x4(&leftSphereRitem->World, leftSphereWorld);
	//	leftSphereRitem->SetTranslation(XMFLOAT3(-5.0f, 3.5f, -10.0f + i * 5.0f));
	//	XMStoreFloat4x4(&leftSphereRitem->TexTransform, BrickTexTransform);
	//	leftSphereRitem->ObjectConstantBufferIndex = objCBIndex++;
	//	leftSphereRitem->Geometry = Geometries["Shape"].get();
	//	leftSphereRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//	leftSphereRitem->IndexCount = leftSphereRitem->Geometry->DrawArgs["Sphere"].IndexCount;
	//	leftSphereRitem->StartIndexLocation = leftSphereRitem->Geometry->DrawArgs["Sphere"].StartIndexLocation;
	//	leftSphereRitem->BaseVertexLocation = leftSphereRitem->Geometry->DrawArgs["Sphere"].BaseVertexLocation;

	//	//XMStoreFloat4x4(&rightSphereRitem->World, rightSphereWorld);
	//	rightSphereRitem->SetTranslation(XMFLOAT3(+5.0f, 3.5f, -10.0f + i * 5.0f));
	//	XMStoreFloat4x4(&rightSphereRitem->TexTransform, BrickTexTransform);
	//	rightSphereRitem->ObjectConstantBufferIndex = objCBIndex++;
	//	rightSphereRitem->Geometry = Geometries["Shape"].get();
	//	rightSphereRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//	rightSphereRitem->IndexCount = rightSphereRitem->Geometry->DrawArgs["Sphere"].IndexCount;
	//	rightSphereRitem->StartIndexLocation = rightSphereRitem->Geometry->DrawArgs["Sphere"].StartIndexLocation;
	//	rightSphereRitem->BaseVertexLocation = rightSphereRitem->Geometry->DrawArgs["Sphere"].BaseVertexLocation;

	//	BuildTargetActor.push_back(std::move(leftCylRitem));
	//	BuildTargetActor.push_back(std::move(rightCylRitem));
	//	BuildTargetActor.push_back(std::move(leftSphereRitem));
	//	BuildTargetActor.push_back(std::move(rightSphereRitem));
	//}

	//// All the render items are opaque.
	//for (auto& e : BuildTargetActor)
	//{
	//	OpaqueActors.push_back(e.get());
	//}

	//for (auto& e : BuildTargetActor)
	//{
	//	WorldActors.push_back(move(e));
	//}
}
