#include "FileIO.h"

#include <Windows.h>

#include "String.h"

void ReadFile(std::string FilePath, std::ifstream& FileStream)
{
    FileStream = std::ifstream(FilePath);

    if (!FileStream)
    {
        MessageBox(0, AnsiToWString(FilePath + std::string(" not found.")).c_str(), 0, 0);
        return;
    }
}
