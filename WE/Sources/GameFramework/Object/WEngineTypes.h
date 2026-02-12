#pragma once
#include <variant>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <set>

using namespace DirectX;

using WSourceRef = std::variant<bool*, int*, float*, XMFLOAT3*, std::string*, std::vector<std::string>*, std::set<std::string>*, std::vector<XMFLOAT3>*>;
using WEvalValue = std::variant<bool, int, float, XMFLOAT3, std::string, std::vector<std::string>, std::set<std::string>, std::vector<XMFLOAT3>>;