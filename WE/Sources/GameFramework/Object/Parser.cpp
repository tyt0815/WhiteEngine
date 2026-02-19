#include "Parser.h"
#include "Actor/Actor.h"

std::vector<std::string> SplitPath(const std::string& Path) 
{
    std::vector<std::string> Tokens;
    std::stringstream ss(Path);
    std::string Token;
    while (std::getline(ss, Token, '.')) {
        if (!Token.empty()) Tokens.push_back(Token);
    }
    return Tokens;
}

WObject* ResolveObjectPath(WObject* StartContext, const std::vector<std::string>& PathTokens, std::string& OutFinalKey) 
{
    if (!StartContext || PathTokens.empty()) return StartContext;

    WObject* Current = StartContext;

    for (size_t i = 0; i < PathTokens.size() - 1; ++i) 
    {
        const std::string& Node = PathTokens[i];

        Current = Current->GetWObject(Node);

        if (!Current) return nullptr;
    }

    // 마지막 토큰은 실제 참조할 키값으로 반환
    OutFinalKey = PathTokens.back();
    return Current;
}

WEvalValue WExpressionParser::Evaluate(WObject* Context, const WAttributesMap& Attributes, const std::string& Name, const std::string& DefaultExpression)
{
    auto it = Attributes.find(Name);
    std::string Exp = (it != Attributes.end() && !it->second.empty()) ? it->second : DefaultExpression;

    if (Exp.empty()) return WEvalValue{ false }; // 기본값

    size_t Pos = 0;
    // [중요] 파싱 자체는 여기서 한 번만 수행하여 '실행 로직(람다)'을 미리 생성합니다.
    auto EvalFunc = ParseLogical(Context, Exp, Pos);

    // 파싱된 로직을 즉시 실행하여 결과값 반환
    return EvalFunc();
}

std::function<WEvalValue()> WExpressionParser::Bind(WObject* Context, const WAttributesMap& Attributes, const std::string& Name, const std::string& DefaultExpression)
{
    auto it = Attributes.find(Name);
    std::string Exp = (it != Attributes.end() && !it->second.empty()) ? it->second : DefaultExpression;

    // 식이 비어있으면 false를 기본값으로 리턴하는 람다 반환
    if (Exp.empty()) return []() { return WEvalValue{ false }; };

    size_t Pos = 0;
    // 파싱 수행: 수식의 실행 구조(트리)가 EvalFunc에 담김
    auto EvalFunc = ParseLogical(Context, Exp, Pos);

    // 런타임에는 파싱 과정 없이 EvalFunc()만 호출함
    return [EvalFunc]() {
        try {
            return EvalFunc();
        }
        catch (...) {
            return WEvalValue{ false };
        }
    };
}

std::function<WEvalValue()> WExpressionParser::ParseLogical(WObject* Context, const std::string& Exp, size_t& Pos)
{
    auto Left = ParseExpression(Context, Exp, Pos);

    SkipSpaces(Exp, Pos);
    std::string Op = PeekOperator(Exp, Pos);

    if (Op == "==" || Op == "!=" || Op == "<" || Op == ">" || Op == "<=" || Op == ">=")
    {
        Pos += Op.length();
        auto Right = ParseExpression(Context, Exp, Pos);

        return [Left, Right, Op]() -> WEvalValue {
            return WVariantOp::Compare(Left(), Right(), Op);
        };
    }
    return Left;
}

std::function<WEvalValue()> WExpressionParser::ParseExpression(WObject* Context, const std::string& Exp, size_t& Pos)
{
    auto Left = ParseTerm(Context, Exp, Pos);

    while (Pos < Exp.length()) {
        char Op = Peek(Exp, Pos);
        if (Op != '+' && Op != '-') break;
        Pos++;

        auto Right = ParseTerm(Context, Exp, Pos);
        return [Left, Right, Op]() -> WEvalValue {
            return (Op == '+') ? WVariantOp::Add(Left(), Right()) : WVariantOp::Sub(Left(), Right());
        };
    }
    return Left;
}

std::function<WEvalValue()> WExpressionParser::ParseTerm(WObject* Context, const std::string& Exp, size_t& Pos)
{
    auto Left = ParseFactor(Context, Exp, Pos);

    while (Pos < Exp.length()) {
        char Op = Peek(Exp, Pos);
        if (Op != '*' && Op != '/') break;
        Pos++;

        auto Right = ParseFactor(Context, Exp, Pos);
        return [Left, Right, Op]() -> WEvalValue {
            return (Op == '*') ? WVariantOp::Mul(Left(), Right()) : WVariantOp::Div(Left(), Right());
        };
    }
    return Left;
}

template<typename... Args>
std::variant<Args...> ParseToVariant(const std::string& String, const std::variant<Args...>&)
{
    std::variant<Args...> Result;

    bool bSuccess = ([&]() {
        using T = Args;
        T TempValue;
        if (WValueParser<T>::Parse(String, TempValue)) {
            Result = std::move(TempValue);
            return true; // 성공하면 true 반환 -> 다음 Args는 생략됨
        }
        return false; // 실패하면 false 반환 -> 다음 Args 시도
        }() || ...);

    return Result;
}

std::function<WEvalValue()> WExpressionParser::ParseFactor(WObject* Context, const std::string& Exp, size_t& Pos)
{
    SkipSpaces(Exp, Pos);

    if (Peek(Exp, Pos) == '(') {
        Pos++;
        auto SubExpr = ParseLogical(Context, Exp, Pos); // 다시 최상위부터
        if (Peek(Exp, Pos) == ')') Pos++;
        return SubExpr;
    }

    size_t Start = Pos;
    if (Exp[Start] == '{')
    {
        while (Exp[Pos] != '}')
        {
            ++Pos;
        }
        ++Pos;
    }
    else
    {
        while (Pos == Start || (Pos < Exp.length() && !IsOperator(Exp[Pos]) && Exp[Pos] != '(' && Exp[Pos] != ')')) Pos++;
        if (Peek(Exp, Pos) == '(')
        {
            while (Pos < Exp.length() && Exp[Pos] != ')') Pos++;
            if (Pos < Exp.length()) Pos++;
        }
    }

    std::string Token = Exp.substr(Start, Pos - Start);
    Token.erase(std::remove(Token.begin(), Token.end(), ' '), Token.end());

    // A. 함수 처리
    if (Token.back() == ')') {
        size_t openParen = Token.find('(');
        std::string PathStr = Token.substr(0, openParen);

        std::string FunctionName;
        std::vector<std::string> Tokens = SplitPath(PathStr);
        WObject* FinalContext = ResolveObjectPath(Context, Tokens, FunctionName);

        return [FinalContext, FunctionName]() {
            return FinalContext ? FinalContext->ExecuteWFunction(FunctionName) : WEvalValue{};
        };
    }
    // B. 소스 참조 ($)
    else if (Token[0] == '$') {
        std::string PathStr = Token.substr(1);

        std::string PropName;
        std::vector<std::string> Tokens = SplitPath(PathStr);
        WObject* FinalContext = ResolveObjectPath(Context, Tokens, PropName);

        return [FinalContext, PropName]() {
            if (!FinalContext) return WEvalValue{};
            WSourceRef Ref = FinalContext->GetWPropertyPtr(PropName);
            return WVariantOp::Deref(Ref);
        };
    }
    // C. 일반 값
    else {
        WEvalValue Val = ParseToVariant(Token, WEvalValue{});
        return [Val]() { return Val; };
    }
}

char WExpressionParser::Peek(const std::string& Exp, size_t& Pos)
{
    SkipSpaces(Exp, Pos);
    return (Pos < Exp.length()) ? Exp[Pos] : '\0';
}

std::string WExpressionParser::PeekTwo(const std::string& Exp, size_t Pos)
{
    if (Pos + 1 < Exp.length()) return Exp.substr(Pos, 2);
    if (Pos < Exp.length()) return Exp.substr(Pos, 1);
    return "";
}

bool WExpressionParser::IsOperator(char c)
{
    // 연산자로 쓰일 수 있는 모든 기호 (XMFLOAT3용 {}, 문장용 "" 등은 제외)
    return c == '+' || c == '-' || c == '*' || c == '/' ||
        c == '=' || c == '!' || c == '<' || c == '>';
}

// 다음 토큰이 비교 연산자인지 확인하고 가져옴
std::string WExpressionParser::PeekOperator(const std::string& Exp, size_t Pos)
{
    std::string Op2 = Exp.substr(Pos, 2);
    if (Op2 == "==" || Op2 == "!=" || Op2 == "<=" || Op2 == ">=") return Op2;

    std::string Op1 = Exp.substr(Pos, 1);
    if (Op1 == "<" || Op1 == ">" || Op1 == "+" || Op1 == "-" || Op1 == "*" || Op1 == "/") return Op1;

    return "";
}

void WExpressionParser::SkipSpaces(const std::string& Exp, size_t& Pos)
{
    while (Pos < Exp.length() && isspace(Exp[Pos])) Pos++;
}


