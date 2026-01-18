#pragma once
#include <memory>
#include <utility>

template <typename T>
using TUniquePtr = std::unique_ptr<T>;

template <typename T>
using TSharedPtr = std::shared_ptr<T>;

template <typename T>
using TWeakPtr = std::weak_ptr<T>;

template<typename T, typename... Args>
__forceinline TUniquePtr<T> MakeUnique(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
_NODISCARD __forceinline TSharedPtr<T> MakeShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

#define Cast std::dynamic_pointer_cast