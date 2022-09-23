#pragma once

#include <unordered_map>
template <typename X, typename Y>
using TMap = std::unordered_map<X,Y>;

#include <string>
using FString = std::string;

#include <vector>
template <typename T>
using TArray = std::vector<T>;

struct FVector
{
    float X, Y, Z;
};

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

