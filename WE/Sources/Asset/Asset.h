#pragma once

#include <DirectXMath.h>
#include <string>
#include <vector>

using namespace DirectX;

class FAsset
{
public:
	FAsset() {}
	virtual ~FAsset() {}

	virtual bool LoadAsset(const std::wstring& FilePath) = 0;

private:
	std::wstring mName;

	friend class FAssetManager;
};