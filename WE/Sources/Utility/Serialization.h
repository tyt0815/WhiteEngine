#include <vector>
#include <string>
#include <cstring>

class FBinaryWriter 
{
public:
    FBinaryWriter(std::vector<unsigned char>& InBuffer) : Buffer(InBuffer) {}

    template<typename T>
    void WriteRaw(const T* Data, size_t Count) {
        static_assert(std::is_trivially_copyable_v<T>, "This type cannot be copied raw.");

        if (Count == 0) return;

        const unsigned char* Ptr = reinterpret_cast<const unsigned char*>(Data);
        Buffer.insert(Buffer.end(), Ptr, Ptr + (sizeof(T) * Count));
    }

    // 기본 타입 (int, float, bool 등)
    template<typename T>
    FBinaryWriter& operator<<(const T& Value) 
    {
        static_assert(std::is_arithmetic_v<T>, "Use specialized logic for non-arithmetic types.");
        const unsigned char* Ptr = reinterpret_cast<const unsigned char*>(&Value);
        Buffer.insert(Buffer.end(), Ptr, Ptr + sizeof(T));
        return *this;
    }

    template<typename T>
    FBinaryWriter& operator<<(const std::vector<T>& Array)
    {
        *this << (int)Array.size();
        for (const auto& Value : Array)
        {
            *this << Value;
        }
        return *this;
    }

    // std::string 특수화 (길이 -> 실제 데이터 순서)
    FBinaryWriter& operator<<(const std::string& Value) 
    {
        uint32_t Length = static_cast<uint32_t>(Value.size());
        *this << Length; // 먼저 4바이트 길이 정보 기록
        Buffer.insert(Buffer.end(), Value.begin(), Value.end());
        return *this;
    }

    FBinaryWriter& operator<<(const char* Value)
    {
        *this << (std::string(Value));
        return *this;
    }

    FBinaryWriter& operator<<(const DirectX::XMFLOAT3& Value)
    {
        *this << Value.x << Value.y << Value.z;
        return *this;
    }

private:
    std::vector<unsigned char>& Buffer;
};

class FBinaryReader 
{
public:
    FBinaryReader(const std::vector<unsigned char>& InBuffer)
        : Buffer(InBuffer), Offset(0) {}

    template<typename T>
    void ReadRaw(T* OutData, size_t Count) {
        static_assert(std::is_trivially_copyable_v<T>, "This type cannot be copied raw.");

        size_t TotalSize = sizeof(T) * Count;
        if (Offset + TotalSize <= Buffer.size()) {
            std::memcpy(OutData, &Buffer[Offset], TotalSize);
            Offset += TotalSize;
        }
    }

    // 기본 타입 (int, float, bool 등)
    template<typename T>
    FBinaryReader& operator>>(T& Value) 
    {
        static_assert(std::is_arithmetic_v<T>, "Use specialized logic for non-arithmetic types.");
        if (Offset + sizeof(T) <= Buffer.size()) {
            std::memcpy(&Value, &Buffer[Offset], sizeof(T));
            Offset += sizeof(T);
        }
        return *this;
    }

    template<typename T>
    FBinaryReader& operator>>(std::vector<T>& Array)
    {
        int Count;
        *this >> Count;
        Array.resize(Count);
        for (int i = 0; i < Count; ++i)
        {
            *this >> Array[i];
        }
        return *this;
    }

    // std::string 특수화
    FBinaryReader& operator>>(std::string& Value) 
    {
        uint32_t Length = 0;
        *this >> Length; // 먼저 4바이트 길이 정보 읽기

        if (Offset + Length <= Buffer.size()) {
            Value.assign(reinterpret_cast<const char*>(&Buffer[Offset]), Length);
            Offset += Length;
        }
        return *this;
    }

    FBinaryReader& operator>>(DirectX::XMFLOAT3& Value)
    {
        *this >> Value.x >> Value.y >> Value.z;
        return *this;
    }

private:
    const std::vector<unsigned char>& Buffer;
    size_t Offset;
};