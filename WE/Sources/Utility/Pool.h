#pragma once
#include <queue>

template <typename T>
class TPool
{
public:
    inline size_t GetNextIndex() const;
    inline size_t Register(T Data);
    inline void Remove(size_t Index);

private:
    std::vector<T> mPool;
    std::vector<bool> mbUsed;
    std::queue<size_t> mIndexQueue;
};

template<typename T>
inline size_t TPool<T>::GetNextIndex() const
{
    return mIndexQueue.empty() ? mPool.size() : mIndexQueue.front();
}

template<typename T>
inline size_t TPool<T>::Register(T Data)
{
    size_t Index = GetNextIndex();
    if (Index < mPool.size())
    {
        mIndexQueue.pop();
        mPool[Index] = Data;
        mbUsed[Index] = true;
    }
    else
    {
        mPool.push_back(Data);
        mbUsed.push_back(true);
    }
    return Index;
}

template<typename T>
inline void TPool<T>::Remove(size_t Index)
{
    mbUsed[Index] = false;
    mIndexQueue.push(Index);
}
