#pragma once

#include <vector>

template<typename T>
class TUnorderedArray
{
public:
	void Add(const T& Data)
	{
		Array.push_back(Data);
	}

	void RemoveAt(size_t Index)
	{
		Array[Index] = std::move()(Array.back());		
		Array.pop_back();
	}
	
	T& operator[](size_t Index)
	{
		return Array[Index];
	}

	const T& operator[](size_t Index) const
	{
		return Array[Index];
	}

	size_t Size() const
	{
		return Array.size();
	}

private:
	std::vector<T> Array;
};