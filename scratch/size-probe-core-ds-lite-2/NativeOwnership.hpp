#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/array.hpp"

class NativeOwnership
{
public:
    virtual ~NativeOwnership() = default;

    template <typename T>
    static void Delete(T value);

    template <typename T>
    static void DeleteItemsAndRelease__ref0(Array<T>*& values);

    template <typename T>
    static void DisposeAndDelete(T value);

    template <typename T>
    static void DisposeAndRelease__ref0(T& value);

    template <typename T>
    static void DisposeItemsAndRelease__ref0(Array<T>*& values);

    template <typename T>
    static void Release__ref0(T& value);
};
