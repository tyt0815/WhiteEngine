#include "XMLDataAsset.h"
#include "AssetLoader.h"
#include "Utility/String.h"

bool FXMLDataAsset::LoadAsset(const std::wstring& FilePath)
{
	return Asset::LoadXML(FilePath, Document);
}
