#include "AssetLoader.h"
#include "Utility/String.h"
#include <sstream>
#include <Windows.h>
#include <zlib.h>
#include <lz4.h>

using namespace tinyxml2;

bool Asset::OpenFile(const std::wstring& FilePath, std::ifstream& File)
{
	if (File.is_open())
	{
		File.close();
	}

	File.open(FilePath);

	if (!File.is_open())
	{
		std::wstringstream ss;
		ss << L"파일을 열 수 없습니다." << FilePath << "\n";
		OutputDebugStringW(ss.str().c_str());
		return false;
	}

	return true;
}

bool Asset::LoadJSON(const std::wstring& FilePath, FJson& Json)
{
	std::ifstream File;

	if (OpenFile(FilePath, File))
	{
		File >> Json;

		return true;
	}

	return false;
}

void foo_recursive(XMLElement* Element)
{

}

bool Asset::LoadXML(const std::string& FilePath, tinyxml2::XMLDocument& Doc)
{
	XMLError Error = Doc.LoadFile(FilePath.c_str());

	if (Error != tinyxml2::XML_SUCCESS)
	{
		std::wstringstream ss;
		ss << L"XML 파일을 열 수 없습니다." << AnsiToWString(FilePath) << "\n";
		OutputDebugStringW(ss.str().c_str());

		return false;
	}

	return true;
}

bool Asset::LoadZlib(const std::string& FilePath, std::vector<unsigned char>& RawBuffer)
{
    // 1. 파일 열기 (바이너리 모드)
    std::ifstream File(AnsiToWString(FilePath), std::ios::binary | std::ios::ate);
    if (!File.is_open())
    {
        return false;
    }

    // 2. 파일 전체 크기 확인 및 데이터 읽기
    std::streamsize FileSize = File.tellg();
    File.seekg(0, std::ios::beg);

    if (FileSize < 4) return false; // 최소한 헤더(4바이트)는 있어야 함

    // 3. 원본 사이즈(Header) 읽기
    uint32_t originalSize = 0;
    File.read(reinterpret_cast<char*>(&originalSize), sizeof(uint32_t));

    // 4. 나머지 압축된 데이터 읽기
    std::streamsize compressedSize = FileSize - sizeof(uint32_t);
    std::vector<unsigned char> compressedBuffer(compressedSize);
    if (!File.read(reinterpret_cast<char*>(compressedBuffer.data()), compressedSize))
    {
        return false;
    }
    File.close();

    // 5. 압축 해제할 결과 버퍼 공간 확보 (+1 바이트 추가)
    RawBuffer.resize(originalSize);

    // 6. zlib uncompress 실행
    uLongf destLen = static_cast<uLongf>(originalSize);
    int result = uncompress(
        static_cast<Bytef*>(RawBuffer.data()),
        &destLen,
        static_cast<const Bytef*>(compressedBuffer.data()),
        static_cast<uLong>(compressedSize)
    );

    // 7. 결과 확인
    if (result != Z_OK)
    {
        // 에러 처리 (Z_MEM_ERROR, Z_BUF_ERROR, Z_DATA_ERROR 등)
        RawBuffer.clear();
        return false;
    }

    return true;
}

bool Asset::LoadZLibXML(const std::string& FilePath, tinyxml2::XMLDocument& Doc)
{
    std::vector<unsigned char> RawBuffer;
    if (!LoadZlib(FilePath, RawBuffer))
    {
        return false;
    }

    XMLError Error = Doc.Parse(reinterpret_cast<const char*>(RawBuffer.data()), RawBuffer.size());
    if (Error != tinyxml2::XML_SUCCESS)
    {
        std::wstringstream ss;
        ss << L"XML 파일을 열 수 없습니다." << AnsiToWString(FilePath) << "\n";
        OutputDebugStringW(ss.str().c_str());

        return false;
    }
    return true;
}

bool Asset::LoadLZ4(const std::string& FilePath, std::vector<unsigned char>& RawBuffer)
{
    // 1. 파일 열기 (바이너리 모드, 끝 지점에서 시작하여 크기 확인)
    std::ifstream File(AnsiToWString(FilePath), std::ios::binary | std::ios::ate);
    if (!File.is_open())
    {
        return false;
    }

    // 2. 파일 전체 크기 확인
    std::streamsize FileSize = File.tellg();
    File.seekg(0, std::ios::beg);

    // 최소 헤더(4바이트 원본크기)는 있어야 함
    if (FileSize < 4) return false;

    // 3. 원본 사이즈(Header) 읽기
    uint32_t originalSize = 0;
    File.read(reinterpret_cast<char*>(&originalSize), sizeof(uint32_t));

    // 4. 압축된 본문 데이터 읽기
    std::streamsize compressedSize = FileSize - sizeof(uint32_t);
    std::vector<char> compressedBuffer(compressedSize);
    if (!File.read(compressedBuffer.data(), compressedSize))
    {
        return false;
    }
    File.close();

    // 5. 결과 버퍼 공간 확보 (+1바이트는 문자열 파싱 대비용 NULL 공간)
    RawBuffer.resize(originalSize + 1);

    // 6. LZ4 해제 실행
    // LZ4_decompress_safe(압축버퍼, 결과버퍼, 압축사이즈, 결과버퍼최대사이즈)
    int decompressResult = LZ4_decompress_safe(
        compressedBuffer.data(),
        reinterpret_cast<char*>(RawBuffer.data()),
        static_cast<int>(compressedSize),
        static_cast<int>(originalSize)
    );

    // 7. 결과 검증
    if (decompressResult < 0)
    {
        // 해제 실패 시 결과값은 음수입니다.
        RawBuffer.clear();
        return false;
    }

    // 마지막 NULL 문자 삽입
    RawBuffer[originalSize] = '\0';

    return true;
}