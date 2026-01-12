#pragma once
#include "Asset.h"

class FSplineDataAsset : public FAsset
{
public:
	virtual bool LoadAsset(const std::wstring& FilePath);

	std::vector<XMFLOAT3> ControlPoints;
	std::vector<XMFLOAT3> LeftHandles;
	std::vector<XMFLOAT3> RightHandles;
	std::vector<float> Property1;
	std::vector<float> Property2;
};