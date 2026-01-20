#pragma once
#include <Windows.h>
#include <Psapi.h>

size_t GetCurrentMemoryUsage() 
{
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    return pmc.PrivateUsage; // 현재 프로세스가 점유한 물리 메모리 (Bytes)
}