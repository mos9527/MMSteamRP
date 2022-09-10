#pragma once
#include <globals.h>
// Signature scan in specified memory region
extern void* sigScan(const char* signature, const char* mask, size_t sigSize, void* memory, size_t memorySize);

// Signature scan in current process
extern void* sigScan(const char* signature, const char* mask, void* hint = nullptr);

#define SIG_SCAN(x, y, ...) \
    inline void* x(); \
    inline void* x##Addr = x(); \
    inline void* x() \
    { \
        constexpr const char* x##Data[] = { __VA_ARGS__ }; \
        constexpr size_t x##Size = _countof(x##Data); \
        if (!x##Addr) \
        { \
            if constexpr (x##Size == 2) \
            { \
                x##Addr = sigScan(x##Data[0], x##Data[1], (void*)(y)); \
                LOG(L"%S found: 0x%llx", #x, (uint64_t)x##Addr); \
                if (x##Addr) \
                    return x##Addr; \
            } \
            else \
            { \
                for (int i = 0; i < x##Size; i += 2) \
                { \
                    x##Addr = sigScan(x##Data[i], x##Data[i + 1], (void*)(y)); \
                    LOG(L"%S found: 0x%llx", #x, (uint64_t)x##Addr); \
                    if (x##Addr) \
                        return x##Addr; \
                } \
            } \
        } \
        return x##Addr; \
    }

#define SIG_SCAN_ADDRESS_ONLY(x,y) SIG_SCAN(x,y,"\x00","?")
