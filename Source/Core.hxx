/*
 * Source/Core.hxx
 *
 * This file is part of the tinyVM source code.
 * Copyright 2015 Patrick L. Melo <patrick@patrickmelo.com.br>
 */

#ifndef VM_CORE_H
#define VM_CORE_H

// C

#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// C++

#include <algorithm>
#include <map>
#include <new>
#include <string>
#include <thread>
#include <vector>

// Only Support C++11 Compilers

#if (__cplusplus < 201103L)
    #error "The compiler does not support the C++11 standard."
#endif

// Architectures

#if defined(i386) || defined(__i386) || defined(__i386__)
    #define X86Arch
    #define ArchName "x86"
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
    #define X64Arch
    #define ArchName "x86_64"
#else
    #error "Unsupported archtecture."
#endif

// Operating Systems

#if defined(__linux__) || defined(__linux) || defined(linux)
    #define LinuxOS
    #define OSName "Linux"
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #define WindowsOS
    #define OSName "Windows"
#else
    #error "Unsupported operating system."
#endif

// Debugging

#ifdef VM_DEBUG
    #define Debug(message, ...) std::printf("\033[1;35m" message "\033[0m\n", ##__VA_ARGS__)
    #define Stub()              std::printf("\033[1;36m[Stub] %s in %s @ %d\033[0m\n", __PRETTY_FUNCTION__, __FILE__, __LINE__)
#else
    #define Debug(message, ...)
    #define Stub(message, ...)
#endif

#define Info(message, ...)    std::printf("\033[1;32m" message "\033[0m\n", ##__VA_ARGS__);
#define Warning(message, ...) std::printf("\033[1;33m" message "\033[0m\n", ##__VA_ARGS__);
#define Error(message, ...)   std::printf("\033[1;31m" message "\033[0m\n", ##__VA_ARGS__);

namespace tinyVM {
// Integer Types

typedef unsigned int uint;
typedef uint8_t      uint8;
typedef uint16_t     uint16;
typedef uint32_t     uint32;
typedef uint64_t     uint64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

// String Types

typedef unsigned char uchar;
typedef std::string   string;
typedef char*         cstring;
typedef char const*   charconst;

// Pointer Types

typedef uint8* buffer;
typedef void*  pointer;

// Buffers

static inline buffer NewBuffer(const uint size) {
    return size > 0 ? new (std::nothrow) uint8[size] : NULL;
}

static inline void DeleteBuffer(buffer& buf) {
    if (buf) {
        delete[] buf;
        buf = NULL;
    }
}

// Memory

struct Memory {
        int64  size;
        int64  index;
        buffer data;
};

typedef Memory* memory;

static inline memory AllocateMemory(const uint size) {
    memory newMemory = new (std::nothrow) Memory();

    if (newMemory) {
        newMemory->size  = size;
        newMemory->index = 0;
        newMemory->data  = NewBuffer(size);

        if (!newMemory->data) {
            delete newMemory;
            return NULL;
        }
    }

    return newMemory;
}

static inline bool ExpandMemory(memory& mem, const int newSize) {
    if (!mem)
        return false;

    buffer newData = NewBuffer(newSize);

    if (!newData)
        return false;

    memcpy(newData, mem->data, mem->size);
    DeleteBuffer(mem->data);

    mem->size = newSize;
    mem->data = newData;

    return true;
}

static inline void DeleteMemory(memory& mem) {
    if (mem) {
        DeleteBuffer(mem->data);
        delete mem;
        mem = NULL;
    }
}

// Strings

static inline cstring NewCString(const uint size) {
    cstring str = new (std::nothrow) char[size + 1];

    if (str != NULL) {
        str[size] = '\0';
    }

    return str;
}

static inline void DeleteCString(cstring& str) {
    if (str) {
        delete[] str;
        str = NULL;
    }
}

static inline int64 ToInt(const string str) {
    return std::atol(str.c_str());
}

static inline double ToFloat(const string str) {
    return std::atof(str.c_str());
}

static inline bool ToBool(const string str) {
    return ((str == "1") || (str == "true") || (str == "on") || (str == "yes"));
}

static inline string FromInt(const int64 value) {
    static char fromBuffer[30];

#ifdef X64Arch
    sprintf(fromBuffer, "%ld", value);
#else
    sprintf(fromBuffer, "%lld", value);
#endif

    return fromBuffer;
}

static inline string FromFloat(const double value, const int precision = -1) {
    static char fromBuffer[100];

    switch (precision) {
        case 1: sprintf(fromBuffer, "%.1f", value); break;
        case 2: sprintf(fromBuffer, "%.2f", value); break;
        case 3: sprintf(fromBuffer, "%.3f", value); break;
        case 4: sprintf(fromBuffer, "%.4f", value); break;
        case 5: sprintf(fromBuffer, "%.5f", value); break;
        case 6: sprintf(fromBuffer, "%.6f", value); break;
        case 7: sprintf(fromBuffer, "%.7f", value); break;
        case 8: sprintf(fromBuffer, "%.8f", value); break;
        default: sprintf(fromBuffer, "%f", value); break;
    }

    return fromBuffer;
}

static inline string FromBool(const bool value) {
    return value ? "true" : "false";
}

static inline void ToUpperCase(string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

static inline void ToLowerCase(string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

// Variable Type Values

struct Value {
        enum {
            Int,
            Float,
            Bool,
            String
        } type;

        int64 size;

        union {
                int64   asInt;
                double  asFloat;
                bool    asBool;
                cstring asString;
        };
};

typedef Value* val;

static inline val NewStringValue(const string value) {
    val newValue = new (std::nothrow) Value();

    if (newValue) {
        newValue->type     = Value::String;
        newValue->size     = value.size();
        newValue->asString = NewCString(newValue->size);

        if (!newValue->asString) {
            delete newValue;
            return NULL;
        }

        memcpy(newValue->asString, value.c_str(), newValue->size);
    }

    return newValue;
}

static inline val NewIntValue(const int64 value) {
    val newValue = new (std::nothrow) Value();

    if (newValue) {
        newValue->type  = Value::Int;
        newValue->size  = sizeof(int64);
        newValue->asInt = value;
    }

    return newValue;
}

static inline val NewFloatValue(const double value) {
    val newValue = new (std::nothrow) Value();

    if (newValue) {
        newValue->type    = Value::Float;
        newValue->size    = sizeof(double);
        newValue->asFloat = value;
    }

    return newValue;
}

static inline val NewBoolValue(const bool value) {
    val newValue = new (std::nothrow) Value();

    if (newValue) {
        newValue->type   = Value::Bool;
        newValue->size   = sizeof(bool);
        newValue->asBool = value;
    }

    return newValue;
}

static inline void DeleteValue(val& value) {
    if (value) {
        if (value->type == Value::String)
            DeleteCString(value->asString);

        delete value;
        value = NULL;
    }
}
};    // namespace tinyVM

// Platform Specific Definitions

#ifdef WindowsOS
    #ifdef NTDDI_VERSION
        #undef NTDDI_VERSION
    #endif

    #ifdef _WIN32_WINNT
        #undef _WIN32_WINNT
    #endif

    #define NTDDI_VERSION NTDDI_WIN7    // Windows 7+
    #define _WIN32_WINNT  _WIN32_WINNT_WIN7

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

extern "C" {
    #include <windows.h>
    #include <windowsx.h>
}
#endif

#endif    // VM_CORE_H
