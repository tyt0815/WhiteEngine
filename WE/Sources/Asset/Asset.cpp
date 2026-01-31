#include "Asset.h"
#include <filesystem>
#include "Utility/FileIO.h"

bool FAssetCompiler::SmartLoad(const std::wstring& SourcePath, std::vector<unsigned char>& OutBuffer)
{
    std::wstring BinaryPath = SourcePath + L"bin";

    // 살짝 보강한다면
    if (CheckIfNeedCompile(SourcePath, BinaryPath))
    {
        std::vector<unsigned char> Buffer;
        if (!OnCompile(SourcePath, Buffer))
        {
            return false;
        }
        FileIO::SaveBufferToFile(BinaryPath, Buffer);
    }

    return FileIO::LoadBufferFromFile(BinaryPath, OutBuffer);
}

bool FAssetCompiler::CheckIfNeedCompile(const std::wstring& Src, const std::wstring& Bin)
{
    if (!std::filesystem::exists(Bin)) return true;
    return std::filesystem::last_write_time(Src) > std::filesystem::last_write_time(Bin);
}