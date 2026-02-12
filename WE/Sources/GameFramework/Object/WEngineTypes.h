#pragma once
#include <variant>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <set>
#include <type_traits>
#include <algorithm>

using namespace DirectX;

// 파싱에 대해 엄격한것 -> 느슨한 순서로 작성할 것.
using WSourceRef = std::variant<std::vector<XMFLOAT3>*, std::vector<std::string>*,  XMFLOAT3*,  bool*,  float*, std::string*,   std::set<std::string>*>;
using WEvalValue = std::variant<std::vector<XMFLOAT3>,  std::vector<std::string>,   XMFLOAT3,   bool,   float,  std::string,    std::set<std::string>>;

namespace DirectX
{
    inline XMFLOAT3 operator+(const XMFLOAT3& a, const XMFLOAT3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
    inline XMFLOAT3 operator-(const XMFLOAT3& a, const XMFLOAT3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
    inline XMFLOAT3 operator*(const XMFLOAT3& a, const XMFLOAT3& b) { return { a.x * b.x, a.y * b.y, a.z * b.z }; }
    inline XMFLOAT3 operator/(const XMFLOAT3& a, const XMFLOAT3& b) { return { a.x / b.x, a.y / b.y, a.z / b.z }; }
    inline XMFLOAT3 operator*(const XMFLOAT3& a, const float& b) { return { a.x * b, a.y * b, a.z * b }; }
    inline XMFLOAT3 operator/(const XMFLOAT3& a, const float& b) { return { a.x / b, a.y / b, a.z / b }; }
    inline bool operator==(const XMFLOAT3& a, const XMFLOAT3& b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
    inline bool operator!=(const XMFLOAT3& a, const XMFLOAT3& b) { return !(a == b); }
}

// [SFINAE 헬퍼] 특정 타입 T가 연산자를 지원하는지 체크
template <typename T, typename = void> struct can_add : std::false_type {};
template <typename T> struct can_add<T, std::void_t<decltype(std::declval<T>() + std::declval<T>())>> : std::true_type {};

template <typename T, typename = void> struct can_sub : std::false_type {};
template <typename T> struct can_sub<T, std::void_t<decltype(std::declval<T>() - std::declval<T>())>> : std::true_type {};

template <typename T, typename = void> struct can_mul : std::false_type {};
template <typename T> struct can_mul<T, std::void_t<decltype(std::declval<T>()* std::declval<T>())>> : std::true_type {};

template <typename T, typename = void> struct can_div : std::false_type {};
template <typename T> struct can_div<T, std::void_t<decltype(std::declval<T>() / std::declval<T>())>> : std::true_type {};

template <typename T, typename = void> struct can_mod : std::false_type {};
template <typename T> struct can_mod<T, std::void_t<decltype(std::declval<T>() % std::declval<T>())>> : std::true_type {};

// --- 비교 연산 (Comparison) ---
template <typename T, typename = void> struct can_eq : std::false_type {};
template <typename T> struct can_eq<T, std::void_t<decltype(std::declval<T>() == std::declval<T>())>> : std::true_type {};

template <typename T, typename = void> struct can_less : std::false_type {};
template <typename T> struct can_less<T, std::void_t<decltype(std::declval<T>() < std::declval<T>())>> : std::true_type {};

// (보통 ==와 <만 있으면 나머지는 추론 가능하지만, 명확성을 위해 추가)
template <typename T, typename = void> struct can_greater : std::false_type {};
template <typename T> struct can_greater<T, std::void_t<decltype(std::declval<T>() > std::declval<T>())>> : std::true_type {};

// --- 논리 연산 (Logical - 주로 bool용) ---
template <typename T, typename = void> struct can_and : std::false_type {};
template <typename T> struct can_and<T, std::void_t<decltype(std::declval<T>() && std::declval<T>())>> : std::true_type {};

template <typename T, typename = void> struct can_or : std::false_type {};
template <typename T> struct can_or<T, std::void_t<decltype(std::declval<T>() || std::declval<T>())>> : std::true_type {};

template <typename T, typename = void> struct can_not : std::false_type {};
template <typename T> struct can_not<T, std::void_t<decltype(!std::declval<T>())>> : std::true_type {};

template <typename T>
struct is_vector : std::false_type {};

// 2. std::vector 일 때만 true가 되도록 특수화
template <typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc>> : std::true_type {};

struct WVariantOp
{
    static WEvalValue Deref(WSourceRef Ref)
    {
        return std::visit([](auto&& r) -> WEvalValue 
            {
                if (r)
                {
                    return *r;
                }
                else
                {
                    return false;
                }
            }, Ref);
    }

    // 1. 더하기 (문자열 연결 포함)
    static WEvalValue Add(const WEvalValue& lhs, const WEvalValue& rhs) {
        return std::visit([](auto&& l, auto&& r) -> WEvalValue {
            using LType = std::decay_t<decltype(l)>;
            using RType = std::decay_t<decltype(r)>;

            if constexpr (std::is_same<LType, RType>::value && !std::is_same<LType, bool>::value) {
                if constexpr (can_add<LType>::value) {
                    // l, r이 0인지 체크하는 로직을 여기에 추가하면 더 안전합니다.
                    return l + r;
                }
            }
            else if constexpr (std::is_arithmetic_v<LType> && std::is_arithmetic_v<RType>) {
                return static_cast<float>(l) + static_cast<float>(r);
            }
            return l;
            }, lhs, rhs);
    }

    // 2. 빼기 (XMFLOAT3 및 산술 타입 전용)
    static WEvalValue Sub(const WEvalValue& lhs, const WEvalValue& rhs) {
        return std::visit([](auto&& l, auto&& r) -> WEvalValue {
            using LType = std::decay_t<decltype(l)>;
            using RType = std::decay_t<decltype(r)>;

            if constexpr (std::is_same<LType, RType>::value && !std::is_same<LType, bool>::value) {
                if constexpr (can_sub<LType>::value) {
                    // l, r이 0인지 체크하는 로직을 여기에 추가하면 더 안전합니다.
                    return l - r;
                }
            }
            else if constexpr (std::is_arithmetic_v<LType> && std::is_arithmetic_v<RType>) {
                return static_cast<float>(l) - static_cast<float>(r);
            }
            return l;
            }, lhs, rhs);
    }

    // 3. 곱하기 (float3 * float 지원)
    static WEvalValue Mul(const WEvalValue& lhs, const WEvalValue& rhs) {
        return std::visit([](auto&& l, auto&& r) -> WEvalValue {
            using LType = std::decay_t<decltype(l)>;
            using RType = std::decay_t<decltype(r)>;

            if constexpr (std::is_same<LType, RType>::value && !std::is_same<LType, bool>::value) {
                if constexpr (can_mul<LType>::value) {
                    // l, r이 0인지 체크하는 로직을 여기에 추가하면 더 안전합니다.
                    return l * r;
                }
            }
            else if constexpr (std::is_arithmetic_v<LType> && std::is_arithmetic_v<RType>) {
                return static_cast<float>(l) * static_cast<float>(r);
            }
            else if constexpr (std::is_same<LType, XMFLOAT3>::value && std::is_arithmetic_v<RType>)
            {
                return l * static_cast<float>(r);
            }
            return l;
            }, lhs, rhs);
    }

    static WEvalValue Div(const WEvalValue& lhs, const WEvalValue& rhs) {
        return std::visit([](auto&& l, auto&& r) -> WEvalValue {
            using LType = std::decay_t<decltype(l)>;
            using RType = std::decay_t<decltype(r)>;

            if constexpr (std::is_same<LType, RType>::value && !std::is_same<LType, bool>::value) {
                if constexpr (can_div<LType>::value) {
                    // l, r이 0인지 체크하는 로직을 여기에 추가하면 더 안전합니다.
                    return l / r;
                }
            }
            else if constexpr (std::is_arithmetic_v<LType> && std::is_arithmetic_v<RType>) {
                return static_cast<float>(l) / static_cast<float>(r);
            }
            else if constexpr (std::is_same<LType, XMFLOAT3>::value && std::is_arithmetic_v<RType>)
            {
                return l / static_cast<float>(r);
            }
            return l;
            }, lhs, rhs);
    }

    static bool Compare(const WEvalValue& lhs, const WEvalValue& rhs, const std::string& Op) {
        return std::visit([&Op](auto&& l, auto&& r) -> bool {
            using LType = std::decay_t<decltype(l)>;
            using RType = std::decay_t<decltype(r)>;

            if constexpr (std::is_same<LType, RType>::value) {
                // 1. 동등 비교 (==, !=)
                if (Op == "==" || Op == "!=") {
                    if constexpr (can_eq<LType>::value) {
                        return (Op == "==") ? (l == r) : !(l == r);
                    }
                    return false;
                }

                // 2. 크기 비교 (<, >, <=, >=)
                // 여기서 can_less<LType>::value가 false이면 아래 블록은 아예 컴파일되지 않음
                if constexpr (can_less<LType>::value && !is_vector<LType>::value) {
                    if (Op == "<")  return l < r;
                    if (Op == ">")  return r < l;
                    if (Op == "<=") return !(r < l);
                    if (Op == ">=") return !(l < r);
                }
                return false;
            }
            else if constexpr (std::is_arithmetic_v<LType> && std::is_arithmetic_v<RType>) {
                // 둘 다 숫자로 변환해서 비교하면 bool vs float 경고가 사라집니다.
                double valL = static_cast<double>(l);
                double valR = static_cast<double>(r);

                if (Op == "==") return valL == valR;
                if (Op == "!=") return valL != valR;
                if (Op == "<")  return valL < valR;
                if (Op == ">")  return valL > valR;
                if (Op == "<=") return valL <= valR;
                if (Op == ">=") return valL >= valR;
                return false;
            }

            return false;
            }, lhs, rhs);
    }
};