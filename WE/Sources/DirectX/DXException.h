#pragma once

#include <string>
#include <Windows.h>

#include "Utility/String.h"

class FDXException
{
public:
    FDXException() = default;
    FDXException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

    std::wstring ToString()const;

    HRESULT ErrorCode = S_OK;
    std::wstring FunctionName;
    std::wstring Filename;
    int LineNumber = -1;
};

#define THROW_IF_FAILED(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw FDXException(hr__, L#x, wfn, __LINE__); } \
}