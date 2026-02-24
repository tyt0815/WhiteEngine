#include "Asset.h"
#include <filesystem>
#include "Utility/FileIO.h"

bool CheckIfNeedCompile(const std::wstring& Src, const std::wstring& Bin)
{
	if (!std::filesystem::exists(Bin)) return true;
	return std::filesystem::last_write_time(Src) > std::filesystem::last_write_time(Bin);
}
