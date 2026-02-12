#pragma once
#include "WEngineTypes.h"

inline XMFLOAT3 operator+(const XMFLOAT3& a, const XMFLOAT3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
inline XMFLOAT3 operator-(const XMFLOAT3& a, const XMFLOAT3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
inline XMFLOAT3 operator*(const XMFLOAT3& a, const XMFLOAT3& b) { return { a.x * b.x, a.y * b.y, a.z * b.z }; }
inline XMFLOAT3 operator/(const XMFLOAT3& a, const XMFLOAT3& b) { return { a.x / b.x, a.y / b.y, a.z / b.z }; }

template<typename T> struct WValueParser;
template<> struct WValueParser<bool>
{
	static bool Parse(const std::string& String)
	{
		std::string LowerStr = String;
		std::transform(LowerStr.begin(), LowerStr.end(), LowerStr.begin(), ::tolower);

		if (LowerStr == "true" || LowerStr == "1") return true;
		if (LowerStr == "false" || LowerStr == "0") return false;
		ReportParseError("Bool (true/false/1/0)", String);
		return false;
	}
};
template<> struct WValueParser<int>
{
	static int Parse(const std::string& String)
	{
		try {
			return std::stoi(String);
		}
		catch (...) {
			ReportParseError("Int", String);
			return 0;
		}
	}
};
template<> struct WValueParser<float>
{
	static float Parse(const std::string& String)
	{
		try {
			return std::stof(String);
		}
		catch (...) {
			ReportParseError("Float", String);
			return 0.0f;
		}
	}
};
template<> struct WValueParser<XMFLOAT3>
{
	static XMFLOAT3 Parse(const std::string& String)
	{
		XMFLOAT3 Float3 = { 0.f, 0.f, 0.f };

		int Result = sscanf_s(String.c_str(), "{%f, %f, %f}", &Float3.x, &Float3.y, &Float3.z);

		if (Result < 3)
		{
			ReportParseError("Float3 {x, y, z}", String);
		}
		return Float3;
	}
};
template<> struct WValueParser<std::string>
{
	static std::string Parse(const std::string& String)
	{
		return String;
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
	static std::vector<T> Parse(const std::string& String)
	{
		std::vector<T> Result;
		std::string CleanStr = String;

		// 1. 앞뒤 공백 제거 및 가장 바깥쪽 대괄호 제거
		CleanStr.erase(0, CleanStr.find_first_not_of(" \t"));
		CleanStr.erase(CleanStr.find_last_not_of(" \t") + 1);

		if (CleanStr.front() == '[') CleanStr.erase(0, 1);
		if (CleanStr.back() == ']') CleanStr.pop_back();

		if (CleanStr.empty()) return Result;

		// 2. 안전한 분리 로직 사용
		std::vector<std::string> tokens = SplitByCommaSafe(CleanStr);

		for (auto& token : tokens)
		{
			// Trim
			token.erase(0, token.find_first_not_of(" \t"));
			token.erase(token.find_last_not_of(" \t") + 1);

			if (!token.empty())
				Result.push_back(WValueParser<T>::Parse(token));
		}
		return Result;
	}
};

template<typename T>
struct WValueParser<std::set<T>>
{
	static std::set<T> Parse(const std::string& String)
	{
		std::set<T> Result;
		if (String.empty()) return Result;

		std::string CleanStr = String;
		CleanStr.erase(std::remove(CleanStr.begin(), CleanStr.end(), '['), CleanStr.end());
		CleanStr.erase(std::remove(CleanStr.begin(), CleanStr.end(), ']'), CleanStr.end());

		std::stringstream ss(CleanStr);
		std::string Token;
		while (std::getline(ss, Token, ','))
		{
			Token.erase(0, Token.find_first_not_of(" "));
			size_t last = Token.find_last_not_of(" ");
			if (last != std::string::npos) Token.erase(last + 1);

			if (!Token.empty())
			{
				Result.insert(WValueParser<T>::Parse(Token));
			}
		}
		return Result;
	}
};

template <typename T>
bool ExtractAttribute(const std::unordered_map<std::string, std::string>& Attrs, const std::string& Key, T& Target)
{
	auto it = Attrs.find(Key);
	if (it != Attrs.end())
	{
		Target = WValueParser<T>::Parse(it->second);
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
		Setter(WValueParser<T>::Parse(it->second));
	}
}

template <typename T, typename TSetterFunc>
void ApplyAttribute(const std::unordered_map<std::string, std::string>& Attrs, const std::string& Key, const T& DefaultValue, TSetterFunc Setter)
{
	auto it = Attrs.find(Key);
	T Value;
	if (it != Attrs.end())
	{
		Value = WValueParser<T>::Parse(it->second);
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
	template<typename T>
	static std::function<T()> Parse(AActor* Context, const WAttributesMap& Attributes, const std::string& Name, const std::string& DefaultExpression)
	{
		auto it = Attributes.find(Name);

		std::string TargetExpression = (it != Attributes.end() && !it->second.empty()) ? it->second : DefaultExpression;
		if (TargetExpression.empty()) return []() { return T{}; };

		// [중요] if constexpr을 사용하여 bool, string, vector 등은 수식 파서 생성을 원천 차단
		if constexpr (std::is_same_v<T, bool> ||
			std::is_same_v<T, std::string> ||
			std::is_same_v<T, std::vector<std::string>> ||
			std::is_same_v<T, std::set<std::string>>) // 컨테이너들도 포함
		{
			// 1. 함수/프로퍼티 체크
			if (TargetExpression.back() == ')') {
				return [Context, TargetExpression]() { return Context->ExecuteWFunction<T>(TargetExpression); };
			}
			if (TargetExpression[0] == '*') {
				std::string PropName = TargetExpression.substr(1);
				return [Context, PropName]() { return *std::get<T*>(Context->GetWPropertyPtr(PropName)); };
			}

			// 2. 단순 값 파싱
			T Value = WValueParser<T>::Parse(TargetExpression);
			return [Value]() { return Value; };
		}
		else
		{
			// [이 부분] T가 산술 연산이 가능한 타입(float, XMFLOAT3 등)일 때만 이 코드가 컴파일됨
			size_t Pos = 0;
			try {
				return ParseExpression<T>(Context, TargetExpression, Pos);
			}
			catch (...) {
				return []() { return T{}; };
			}
		}
	}


private:
	// 1. 더하기/빼기 (가장 낮은 우선순위)
	template<typename T>
	static std::function<T()> ParseExpression(AActor* Context, const std::string& Exp, size_t& Pos)
	{
		auto Left = ParseTerm<T>(Context, Exp, Pos);

		while (Pos < Exp.length()) {
			char Op = Peek(Exp, Pos);
			if (Op != '+' && Op != '-') break;
			Pos++; // 연산자 소비

			auto Right = ParseTerm<T>(Context, Exp, Pos);
			if (Op == '+')
				Left = [Left, Right]() { return Left() + Right(); };
			else
				Left = [Left, Right]() { return Left() - Right(); };
		}
		return Left;
	}

	// 2. 곱하기/나누기
	template<typename T>
	static std::function<T()> ParseTerm(AActor* Context, const std::string& Exp, size_t& Pos)
	{
		std::function<T()> Left = ParseFactor<T>(Context, Exp, Pos);

		while (Pos < Exp.length()) {
			char Op = Peek(Exp, Pos);
			if (Op != '*' && Op != '/') break;
			Pos++;

			std::function<T()> Right = ParseFactor<T>(Context, Exp, Pos);
			if (Op == '*')
				Left = [Left, Right]() { return Left() * Right(); };
			else
				Left = [Left, Right]() { return Left() / Right(); };
		}
		return Left;
	}

	// 3. 최우선 순위 (괄호, 함수, 변수, 값)
	template<typename T>
	static std::function<T()> ParseFactor(AActor* Context, const std::string& Exp, size_t& Pos)
	{
		SkipSpaces(Exp, Pos);

		// A. 괄호 처리
		if (Peek(Exp, Pos) == '(') {
			Pos++; // '(' 소비
			auto SubExpr = ParseExpression<T>(Context, Exp, Pos);
			SkipSpaces(Exp, Pos);
			if (Peek(Exp, Pos) == ')') Pos++; // ')' 소비
			return SubExpr;
		}

		// B. 토큰 추출 (연산자나 괄호를 만나기 전까지)
		size_t Start = Pos;
		while (Pos < Exp.length() && !IsOperator(Exp[Pos]) && Exp[Pos] != '(' && Exp[Pos] != ')') {
			Pos++;
		}

		// 만약 함수라면 ( 예: GetLoc() ) 괄호까지 포함해서 추출
		if (Peek(Exp, Pos) == '(')
		{
			while (Pos < Exp.length() && Exp[Pos] != ')') Pos++;
			if (Pos < Exp.length()) Pos++; // ')'까지 소비
		}

		std::string Token = Exp.substr(Start, Pos - Start);
		Token.erase(std::remove(Token.begin(), Token.end(), ' '), Token.end()); // 공백 제거

		// C. 토큰 성격 판별
		// 1) 함수: 끝이 ) 인 경우
		if (!Token.empty() && Token.back() == ')') {
			return [Context, Token]() {
				return Context->ExecuteWFunction<T>(Token);
			};
		}
		// 2) 프로퍼티: 시작이 $ 인 경우
		else if (!Token.empty() && Token[0] == '$') {
			std::string PropName = Token.substr(1);
			return [Context, PropName]() {
				return *std::get<T*>(Context->GetWPropertyPtr(PropName));
			};
		}
		// 3) 값: { } 또는 숫자 (WPropertyTrait 활용)
		else {
			T Value = WValueParser<T>::Parse(Token);
			return [Value]() { return Value; };
		}
	}

	// 유틸리티 함수들
	static char Peek(const std::string& Exp, size_t& Pos) {
		SkipSpaces(Exp, Pos);
		return (Pos < Exp.length()) ? Exp[Pos] : '\0';
	}

	static bool IsOperator(char c) { return c == '+' || c == '-' || c == '*' || c == '/'; }

	static void SkipSpaces(const std::string& Exp, size_t& Pos) {
		while (Pos < Exp.length() && isspace(Exp[Pos])) Pos++;
	}
};