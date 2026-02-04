#pragma once

#include "Utility/Class.h"
#include "Asset.h"
#include "Utility/Memory.h"
#include <unordered_map>
#include <Windows.h>
#include <sstream>

class FAssetManager final
{
	SINGLETON(FAssetManager);
public:
	void LoadAssets();

	FAsset* GetAsset(const std::string& Name);

	template<typename TAsset>
	static bool LoadAsset(const std::wstring& FilePath, const std::string& AssetName);

	template<typename TAsset>
	static TAsset* GetAsset(const std::string& Name);

private:
	void LoadBlueprints();

	void LoadObjectAnimations();

	void LoadSplines();

	std::unordered_map<std::string, TUniquePtr<FAsset>> mAssets;
};

template<typename TAsset>
inline bool FAssetManager::LoadAsset(const std::wstring& FilePath, const std::string& AssetName)
{
	TUniquePtr<FAsset> Asset = std::make_unique<TAsset>();
	Asset->mName = AssetName;
	if (!Asset->LoadAsset(FilePath))
	{
		assert(false && "Fail to load asset");
		return false;
	}
	GetInstance()->mAssets[AssetName] = std::move(Asset);

	return true;
}

template<typename TAsset>
inline TAsset* FAssetManager::GetAsset(const std::string& Name)
{
	return dynamic_cast<TAsset*>(GetInstance()->GetAsset(Name));
}