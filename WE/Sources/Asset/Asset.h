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

protected:
	std::string mName;

	friend class FAssetManager;
};

class FAssetCompiler {
public:
    virtual ~FAssetCompiler() = default;

    // 공통 로직: 똑똑한 로드 (Template Method)
    bool SmartLoad(const std::wstring& SourcePath, std::vector<unsigned char>& OutBuffer);

protected:
    // 자식에서 반드시 구현해야 할 '진짜' 컴파일 로직
    virtual bool OnCompile(const std::wstring& SrcPath, std::vector<unsigned char>& OutBuffer) = 0;

private:
    bool CheckIfNeedCompile(const std::wstring& Src, const std::wstring& Bin);
};