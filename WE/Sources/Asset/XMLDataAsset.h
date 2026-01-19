#pragma once
#include "Asset.h"
#include <tinyxml2.h>

class FXMLDataAsset : public FAsset
{
public:
	virtual bool LoadAsset(const std::wstring& FilePath) override;
	tinyxml2::XMLDocument Document;
};