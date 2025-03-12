#pragma once
#include <queue>

template <typename T>
class TPool
{
public:
    inline size_t GetNextIndex() const;
    inline size_t Register(T Data);
    inline void Remove(size_t Index);
    inline bool IsUsed(size_t Index) const;
    inline size_t GetPoolSize() const;
    inline T GetItem(size_t Index) const;
    inline T& GetItemRef(size_t Index);
    inline void SetItem(size_t i, const T& Item);

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

template<typename T>
inline bool TPool<T>::IsUsed(size_t Index) const
{
    return mbUsed[Index];
}

template<typename T>
inline size_t TPool<T>::GetPoolSize() const
{
    return mPool.size();
}

template<typename T>
inline T TPool<T>::GetItem(size_t Index) const
{
    return mPool[Index];
}

template<typename T>
inline T& TPool<T>::GetItemRef(size_t Index)
{
    return mPool[Index];
}

template<typename T>
inline void TPool<T>::SetItem(size_t i, const T& Item)
{
    mPool[i] = Item;
}
