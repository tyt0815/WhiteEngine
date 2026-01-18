#pragma once

#include <vector>

template <typename T>
using TArray = std::vector<T>;

//template<typename T>
//class TUnorderedArray
//{
//public:
//	template<typename __T>
//	size_t Add(__T&& Data)
//	{
//		Array.emplace_back(std::move(Data));
//		return Array.size() - 1;
//	}
//
//	void RemoveAt(size_t Index)
//	{
//		Array[Index] = std::move(Array.back());
//		Array.pop_back();
//	}
//	
//	T& operator[](size_t Index)
//	{
//		return Array[Index];
//	}
//
//	const T& operator[](size_t Index) const
//	{
//		return Array[Index];
//	}
//
//	size_t Size() const
//	{
//		return Array.size();
//	}
//
//private:
//	std::vector<T> Array;
//
//public:
//	const std::vector<T>& GetView() const
//	{
//		return Array;
//	}
//
//	T& Back()
//	{
//		return Array.back();
//	}
//};