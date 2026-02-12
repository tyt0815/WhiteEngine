#pragma once
#include <string>
#include <iostream>
#include <DirectXMath.h>

void ShowMessageBox(const std::string& Content);

void ShowMessageBox(const std::wstring& Content);

void Log(const std::string& Content);

void Log(const std::wstring& Content);


using namespace DirectX;

inline std::ostream& operator<<(std::ostream& os, const XMFLOAT3& v)
{
    os << "{" << v.x << ", " << v.y << ", " << v.z << "}";
    return os;
}