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

bool FileIO::SaveBufferToFile(const std::wstring& Path, const std::vector<unsigned char>& Buffer)
{
    // std::ios::binary 플래그가 중요합니다 (개행 문자 변환 방지)
    std::ofstream OutFile(Path, std::ios::binary);

    if (!OutFile.is_open()) return false;

    // 버퍼의 시작 주소부터 크기만큼 한 번에 기록
    OutFile.write(reinterpret_cast<const char*>(Buffer.data()), Buffer.size());

    OutFile.close();
    return true;
}

bool FileIO::LoadBufferFromFile(const std::wstring& Path, std::vector<unsigned char>& OutBuffer)
{
    std::ifstream InFile(Path, std::ios::binary | std::ios::ate); // ate: 열자마자 끝으로 이동

    if (!InFile.is_open()) return false;

    // 2-1. 파일 크기 확인 후 버퍼 공간 확보
    std::streamsize Size = InFile.tellg();
    InFile.seekg(0, std::ios::beg); // 다시 처음으로 이동

    OutBuffer.resize(static_cast<size_t>(Size));

    // 2-2. 한 번에 읽기
    if (InFile.read(reinterpret_cast<char*>(OutBuffer.data()), Size))
    {
        return true;
    }

    return false;
}
