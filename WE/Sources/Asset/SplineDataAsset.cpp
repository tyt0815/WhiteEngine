#include "SplineDataAsset.h"
#include "AssetLoader.h"

using namespace Asset;

bool FSplineDataAsset::LoadAsset(const std::wstring& FilePath)
{
	FJson Json;
	if (!LoadJSON(FilePath, Json))
	{
		return false;
	}

	for (const auto& Item : Json)
	{
		ControlPoints.push_back(XMFLOAT3(Item["ControlPoint"][0], Item["ControlPoint"][1], Item["ControlPoint"][2]));
		LeftHandles.push_back(XMFLOAT3(Item["LeftHandle"][0], Item["LeftHandle"][1], Item["LeftHandle"][2]));
		RightHandles.push_back(XMFLOAT3(Item["RightHandle"][0], Item["RightHandle"][1], Item["RightHandle"][2]));
		Property1.push_back(Item["Property1"]);
		Property2.push_back(Item["Property2"]);
	}

	return true;
}