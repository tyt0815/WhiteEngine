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

	FAsset* GetAsset(const std::wstring& Name);

	template<typename TAsset>
	static bool LoadAsset(const std::wstring& FilePath, const std::wstring& AssetName);

	template<typename TAsset>
	static TAsset* GetAsset(const std::wstring& Name);

private:
	void LoadBlueprints();

	void LoadObjectAnimations();

	std::unordered_map<std::wstring, TUniquePtr<FAsset>> mAssets;
};

template<typename TAsset>
inline bool FAssetManager::LoadAsset(const std::wstring& FilePath, const std::wstring& AssetName)
{
	TUniquePtr<FAsset> Asset = std::make_unique<TAsset>();
	Asset->mName = AssetName;
	if (!Asset->LoadAsset(FilePath))
	{
		std::wstringstream wss;
		wss << L"에셋 로드에 실패하였습니다.: " << AssetName << L"\n";
		OutputDebugStringW(wss.str().c_str());
		assert(false);
		return false;
	}
	GetInstance()->mAssets[AssetName] = std::move(Asset);

	return true;
}

template<typename TAsset>
inline TAsset* FAssetManager::GetAsset(const std::wstring& Name)
{
	return dynamic_cast<TAsset*>(GetInstance()->GetAsset(Name));
}