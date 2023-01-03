#pragma once
#include "Includes.hpp"
#include <functional>

namespace fx
{
    enum FN
    {
        EGETPATH = 5,
        EGETSTATE = 6,
        ELOADFROM = 7,
        ESTART = 8,
        ESTOP = 9,
        ERUN = 10,
    };

    enum ResourceState : int32_t
    {
        Uninitialized = 0,
        Stopped = 1,
        Starting = 2,
        Started = 3,
        Stopping = 4,
    };

    class ResourceManagerImpl;

    class ResourceImpl
    {
    public:
        char pad_000[0xD0];
        std::string m_name;
        std::string m_rootPath;
        ResourceManagerImpl* m_manager;

        const std::basic_string<char, std::char_traits<char>, std::allocator<char>>* GetPath()
        {
            typedef const std::basic_string<char, std::char_traits<char>, std::allocator<char>>* (__fastcall* tGetPath)(fx::ResourceImpl*);
            uintptr_t* pVTableBase = (uintptr_t*)(*(uintptr_t*)this);
            tGetPath GetPathR = (tGetPath)(*(pVTableBase + static_cast<int>(FN::EGETPATH)));
            return GetPathR(this);
        }

        ResourceState GetState()
        {
            typedef ResourceState(__fastcall* tGetState)(fx::ResourceImpl*);
            uintptr_t* pVTableBase = (uintptr_t*)(*(uintptr_t*)this);
            tGetState GetStateR = (tGetState)(*(pVTableBase + static_cast<int>(FN::EGETSTATE)));
            return GetStateR(this);
        }

        bool LoadFrom(const std::basic_string<char, std::char_traits<char>, std::allocator<char>>* rootPath, std::basic_string<char, std::char_traits<char>, std::allocator<char>>* errorState)
        {
            typedef bool(__fastcall* tLoadFrom)(fx::ResourceImpl*, const std::basic_string<char, std::char_traits<char>, std::allocator<char>>*, std::basic_string<char, std::char_traits<char>, std::allocator<char>>*);
            uintptr_t* pVTableBase = (uintptr_t*)(*(uintptr_t*)this);
            tLoadFrom LoadFromR = (tLoadFrom)(*(pVTableBase + static_cast<int>(FN::ELOADFROM)));
            return LoadFromR(this, rootPath, errorState);
        }

        bool Start()
        {
            typedef bool(__fastcall* tStart)(fx::ResourceImpl*);
            uintptr_t* pVTableBase = (uintptr_t*)(*(uintptr_t*)this);
            tStart StartR = (tStart)(*(pVTableBase + static_cast<int>(FN::ESTART)));
            return StartR(this);
        }

        bool Stop()
        {
            typedef bool(__fastcall* tStop)(fx::ResourceImpl*);
            uintptr_t* pVTableBase = (uintptr_t*)(*(uintptr_t*)this);
            tStop StopR = (tStop)(*(pVTableBase + static_cast<int>(FN::ESTOP)));
            return StopR(this);
        }

        void Run(std::function<void __cdecl(void)>* fn)
        {
            typedef void(__fastcall* tRun)(fx::ResourceImpl*, std::function<void __cdecl(void)>*);
            uintptr_t* pVTableBase = (uintptr_t*)(*(uintptr_t*)this);
            tRun RunR = (tRun)(*(pVTableBase + static_cast<int>(FN::ERUN)));
            RunR(this, fn);
        }
    };
}

typedef struct LoadS {
    const char* s;
    size_t size;
} LoadS;

class Vector3 final
{
public:

    float x, y, z;

    Vector3(const float x, const float y, const float z) : x(x), y(y), z(z) {}
    Vector3 operator + (const Vector3& rhs) const { return Vector3(x + rhs.x, y + rhs.y, z + rhs.z); }
    Vector3 operator - (const Vector3& rhs) const { return Vector3(x - rhs.x, y - rhs.y, z - rhs.z); }
    Vector3 operator * (const float& rhs) const { return Vector3(x * rhs, y * rhs, z * rhs); }
    Vector3 operator / (const float& rhs) const { return Vector3(x / rhs, y / rhs, z / rhs); }
    bool operator == (const Vector3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
    Vector3& operator += (const Vector3& rhs) { return *this = *this + rhs; }
    Vector3& operator -= (const Vector3& rhs) { return *this = *this - rhs; }
    Vector3& operator *= (const float& rhs) { return *this = *this * rhs; }
    Vector3& operator /= (const float& rhs) { return *this = *this / rhs; }
    float Length() const { return sqrt(x * x + y * y + z * z); }
    Vector3 Normalize() const { return *this * (1 / Length()); }
    Vector3 Multiple(const Vector3 v) const { return Vector3(x * v.x, y * v.y, z * v.z); }

    float Distance(const Vector3& rhs) const { return (*this - rhs).Length(); }
    void Invert() { *this *= -1; }
    static Vector3 FromM128(__m128 in) { return Vector3(in.m128_f32[0], in.m128_f32[1], in.m128_f32[2]); }
};

class World
{
public:
    char pad_000[0x8];
    class Player* LocalPlayer; // 0x8
};

class Player
{
public:
    char pad_000[0x20]; // 0x00
    class PedModelInfo* pedModelInfo; // 0x020
    char pad_028[0x68]; // 0x028
    Vector3 Coords; // 0x090
    char pad_09C[0xED]; // 0x09C
    bool GodMode; // 0x189
    char pad_001[0xF6]; // 0x18A
    float Health; // 0x280
    char pad_002[0xE34]; // 0x284
    class PlayerInfo* playerInfo; // 0X10B8
    char pad_003[0x8];
    class WeaponManager* WeaponManagerPtr; // 0x10C8
    char pad_004[0x3E8];
    float Armor; // 0x14B8
};

class PedModelInfo
{
public:
    char pad_000[0x30];
    Vector3 Min; // 0x030
    Vector3 Max; // 0x040
};

class PlayerInfo
{
public:
    char pad_000[0x68]; // 0x00
    int Health; // 0x068
};

class ReplayInterface
{
public:
    char pad_000[0x18]; // 0x00
    class PedInterface* pedInterface; // 0x018
};

class PedInterface
{
public:
    char pad_000[0x100]; // 0x00
    class PedList* pedList; // 0x100;
    uint64_t pedMaximum; // 0x108
};

class PedList
{
public:
    Player* GetCurrentPlayer(int index)
    {
        if (!this) return 0;
        return (Player*)(*(uint64_t*)(this + (index * 0x10)));
    }
};

class WeaponManager
{

};