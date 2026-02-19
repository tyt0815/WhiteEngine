#pragma once
#include "WEngineTypes.h"
#include "Asset/BlueprintTypes.h"
#include <functional>

class WObject;

template<typename T> struct WValueParser;
template<> struct WValueParser<bool>
{
	static bool Parse(const std::string& String, bool& Out)
	{
		std::string LowerStr = String;
		std::transform(LowerStr.begin(), LowerStr.end(), LowerStr.begin(), ::tolower);

		if (LowerStr == "true")
		{
			Out = true;
			return true;
		}
		if (LowerStr == "false")
		{
			Out = false;
			return true;
		}
		return false;
	}
};
template<> struct WValueParser<float>
{
	static float Parse(const std::string& String, float& Out)
	{
		try {
			Out = std::stof(String);
			return true;
		}
		catch (...) {
			return false;
		}
	}
};
template<> struct WValueParser<XMFLOAT3>
{
	static bool Parse(const std::string& String, XMFLOAT3& Out)
	{
		XMFLOAT3 Float3 = { 0.f, 0.f, 0.f };

		int Result = sscanf_s(String.c_str(), "{%f, %f, %f}", &Float3.x, &Float3.y, &Float3.z);

		if (Result < 3)
		{
			return false;
		}

		Out = Float3;

		return true;
	}
};

// 유틸리티 함수: 괄호를 고려하여 문자열 분리
static std::vector<std::string> SplitByCommaSafe(const std::string& s) {
	std::vector<std::string> tokens;
	std::string current;
	int braceDepth = 0;
	int bracketDepth = 0;

	for (char c : s) {
		if (c == '{') braceDepth++;
		else if (c == '}') braceDepth--;
		else if (c == '[') bracketDepth++;
		else if (c == ']') bracketDepth--;

		// 괄호 밖에서 쉼표를 만났을 때만 토큰 분리
		if (c == ',' && braceDepth == 0 && bracketDepth == 0) {
			tokens.push_back(current);
			current.clear();
		}
		else {
			current += c;
		}
	}
	if (!current.empty()) tokens.push_back(current);
	return tokens;
}

template<typename T>
struct WValueParser<std::vector<T>>
{
	static bool Parse(const std::string& String, std::vector<T>& Out)
	{
		Out.clear();
		std::string CleanStr = String;

		// 1. 앞뒤 공백 제거 및 가장 바깥쪽 대괄호 제거
		CleanStr.erase(0, CleanStr.find_first_not_of(" \t"));
		CleanStr.erase(CleanStr.find_last_not_of(" \t") + 1);

		if (CleanStr.front() != '[')
		{
			return false;
		}
		CleanStr.erase(0, 1);
		if (CleanStr.back() != ']')
		{
			return false;
		}
		CleanStr.pop_back();

		if (CleanStr.empty()) return true;

		// 2. 안전한 분리 로직 사용
		std::vector<std::string> tokens = SplitByCommaSafe(CleanStr);

		for (auto& token : tokens)
		{
			// Trim
			token.erase(0, token.find_first_not_of(" \t"));
			token.erase(token.find_last_not_of(" \t") + 1);

			if (!token.empty())
			{
				T Value;
				if (WValueParser<T>::Parse(token, Value))
				{
					Out.push_back(Value);
				}
				else
				{
					return false;
				}
			}
				
		}
		return true;
	}
};

template<typename T>
struct WValueParser<std::set<T>>
{
	static bool Parse(const std::string& String, std::set<T>& Out)
	{
		if (String.empty()) return true;

		std::string CleanStr = String;
		CleanStr.erase(0, CleanStr.find_first_not_of(" \t"));
		CleanStr.erase(CleanStr.find_last_not_of(" \t") + 1);

		if (CleanStr.front() != '[')
		{
			return false;
		}
		CleanStr.erase(0, 1);
		if (CleanStr.back() != ']')
		{
			return false;
		}
		CleanStr.pop_back();

		std::stringstream ss(CleanStr);
		std::string Token;
		while (std::getline(ss, Token, ','))
		{
			Token.erase(0, Token.find_first_not_of(" "));
			size_t last = Token.find_last_not_of(" ");
			if (last != std::string::npos) Token.erase(last + 1);

			if (!Token.empty())
			{
				T Value;
				if (WValueParser<T>::Parse(Token, Value))
				{
					Out.insert(Value);
				}
				else
				{
					return false;
				}
			}
		}
		return true;
	}
};

template<> struct WValueParser<std::string>
{
	static bool Parse(const std::string& String, std::string& Out)
	{
		Out = String;
		return true;
	}
};

template <typename T>
bool ExtractAttribute(const std::unordered_map<std::string, std::string>& Attrs, const std::string& Key, T& Target)
{
	auto it = Attrs.find(Key);
	if (it != Attrs.end())
	{
		WValueParser<T>::Parse(it->second, Target);
		return true;
	}
	return false;
}

template <typename T, typename TSetterFunc>
void ApplyAttribute(const std::unordered_map<std::string, std::string>& Attrs, const std::string& Key, TSetterFunc Setter)
{
	auto it = Attrs.find(Key);
	if (it != Attrs.end())
	{
		T Value;
		WValueParser<T>::Parse(it->second, Value);
		Setter(Value);
	}
}

template <typename T, typename TSetterFunc>
void ApplyAttribute(const std::unordered_map<std::string, std::string>& Attrs, const std::string& Key, const T& DefaultValue, TSetterFunc Setter)
{
	auto it = Attrs.find(Key);
	T Value;
	if (it != Attrs.end())
	{
		WValueParser<T>::Parse(it->second, Value);
	}
	else
	{
		Value = DefaultValue;
	}
	Setter(Value);
}

class WExpressionParser
{
public:
	static WEvalValue Evaluate(WObject* Context, const WAttributesMap& Attributes, const std::string& Name, const std::string& DefaultExpression);

	static std::function<WEvalValue()> Bind(WObject* Context, const WAttributesMap& Attributes, const std::string& Name, const std::string& DefaultExpression);

	template<typename T>
	static std::function<T()> Bind(WObject* Context, const WAttributesMap& Attributes, const std::string& Name, const std::string& DefaultExpression)
	{
		// 위에서 만든 WEvalValue 버전의 Bind를 호출
		auto BaseEval = Bind(Context, Attributes, Name, DefaultExpression);

		// 결과를 T로 변환해주는 래퍼 람다 반환
		return [BaseEval]() -> T {
			WEvalValue Result = BaseEval();

			return std::visit([](auto&& arg) -> T {
				using ArgType = std::decay_t<decltype(arg)>;
				if constexpr (std::is_convertible_v<ArgType, T>) {
					return static_cast<T>(arg);
				}
				return T{};
				}, Result);
		};
	}

private:
	static std::function<WEvalValue()> ParseLogical(WObject* Context, const std::string& Exp, size_t& Pos);

	static std::function<WEvalValue()> ParseExpression(WObject* Context, const std::string& Exp, size_t& Pos);

	static std::function<WEvalValue()> ParseTerm(WObject* Context, const std::string& Exp, size_t& Pos);

	static std::function<WEvalValue()> ParseFactor(WObject* Context, const std::string& Exp, size_t& Pos);

	// 유틸리티 함수들
	static char Peek(const std::string& Exp, size_t& Pos);

	static std::string PeekTwo(const std::string& Exp, size_t Pos);

	static bool IsOperator(char c);

	static std::string PeekOperator(const std::string& Exp, size_t Pos);

	static void SkipSpaces(const std::string& Exp, size_t& Pos);
};