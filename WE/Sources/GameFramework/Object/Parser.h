#pragma once
#include "WEngineTypes.h"
#include "Asset/BlueprintTypes.h"
#include <functional>

class AActor;

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

		Out = std::move(Float3);

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
	static WEvalValue Evaluate(AActor* Context, const WAttributesMap& Attributes, const std::string& Name, const std::string& DefaultExpression);

	static std::function<WEvalValue()> Bind(AActor* Context, const WAttributesMap& Attributes, const std::string& Name, const std::string& DefaultExpression);

	template<typename T>
	static std::function<T()> Bind(AActor* Context, const WAttributesMap& Attributes, const std::string& Name, const std::string& DefaultExpression)
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
	static std::function<WEvalValue()> ParseLogical(AActor* Context, const std::string& Exp, size_t& Pos);

	static std::function<WEvalValue()> ParseExpression(AActor* Context, const std::string& Exp, size_t& Pos);

	static std::function<WEvalValue()> ParseTerm(AActor* Context, const std::string& Exp, size_t& Pos);

	static std::function<WEvalValue()> ParseFactor(AActor* Context, const std::string& Exp, size_t& Pos);

	// 유틸리티 함수들
	static char Peek(const std::string& Exp, size_t& Pos);

	static std::string PeekTwo(const std::string& Exp, size_t Pos);

	static bool IsOperator(char c);

	static std::string PeekOperator(const std::string& Exp, size_t Pos);

	static void SkipSpaces(const std::string& Exp, size_t& Pos);

	//// 1. 더하기/빼기 (가장 낮은 우선순위)
	//template<typename T>
	//static std::function<T()> ParseExpression(AActor* Context, const std::string& Exp, size_t& Pos)
	//{
	//	auto Left = ParseTerm<T>(Context, Exp, Pos);

	//	while (Pos < Exp.length()) {
	//		char Op = Peek(Exp, Pos);
	//		if (Op != '+' && Op != '-') break;
	//		Pos++; // 연산자 소비

	//		auto Right = ParseTerm<T>(Context, Exp, Pos);
	//		if (Op == '+')
	//			Left = [Left, Right]() { return Left() + Right(); };
	//		else
	//			Left = [Left, Right]() { return Left() - Right(); };
	//	}
	//	return Left;
	//}

	//// 2. 곱하기/나누기
	//template<typename T>
	//static std::function<T()> ParseTerm(AActor* Context, const std::string& Exp, size_t& Pos)
	//{
	//	std::function<T()> Left = ParseFactor<T>(Context, Exp, Pos);

	//	while (Pos < Exp.length()) {
	//		char Op = Peek(Exp, Pos);
	//		if (Op != '*' && Op != '/') break;
	//		Pos++;

	//		std::function<T()> Right = ParseFactor<T>(Context, Exp, Pos);
	//		if (Op == '*')
	//			Left = [Left, Right]() { return Left() * Right(); };
	//		else
	//			Left = [Left, Right]() { return Left() / Right(); };
	//	}
	//	return Left;
	//}

	//// 3. 최우선 순위 (괄호, 함수, 변수, 값)
	//template<typename T>
	//static std::function<T()> ParseFactor(AActor* Context, const std::string& Exp, size_t& Pos)
	//{
	//	SkipSpaces(Exp, Pos);

	//	// A. 괄호 처리
	//	if (Peek(Exp, Pos) == '(') {
	//		Pos++; // '(' 소비
	//		auto SubExpr = ParseExpression<T>(Context, Exp, Pos);
	//		SkipSpaces(Exp, Pos);
	//		if (Peek(Exp, Pos) == ')') Pos++; // ')' 소비
	//		return SubExpr;
	//	}

	//	// B. 토큰 추출 (연산자나 괄호를 만나기 전까지)
	//	size_t Start = Pos;
	//	while (Pos < Exp.length() && !IsOperator(Exp[Pos]) && Exp[Pos] != '(' && Exp[Pos] != ')') {
	//		Pos++;
	//	}

	//	// 만약 함수라면 ( 예: GetLoc() ) 괄호까지 포함해서 추출
	//	if (Peek(Exp, Pos) == '(')
	//	{
	//		while (Pos < Exp.length() && Exp[Pos] != ')') Pos++;
	//		if (Pos < Exp.length()) Pos++; // ')'까지 소비
	//	}

	//	std::string Token = Exp.substr(Start, Pos - Start);
	//	Token.erase(std::remove(Token.begin(), Token.end(), ' '), Token.end()); // 공백 제거

	//	// C. 토큰 성격 판별
	//	// 1) 함수: 끝이 ) 인 경우
	//	if (!Token.empty() && Token.back() == ')') {
	//		return [Context, Token]() {
	//			return Context->ExecuteWFunction<T>(Token);
	//		};
	//	}
	//	// 2) 프로퍼티: 시작이 $ 인 경우
	//	else if (!Token.empty() && Token[0] == '$') {
	//		std::string PropName = Token.substr(1);
	//		return [Context, PropName]() {
	//			return *std::get<T*>(Context->GetWPropertyPtr(PropName));
	//		};
	//	}
	//	// 3) 값: { } 또는 숫자 (WPropertyTrait 활용)
	//	else {
	//		T Value = WValueParser<T>::Parse(Token);
	//		return [Value]() { return Value; };
	//	}
	//}
};