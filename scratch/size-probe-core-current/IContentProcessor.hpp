#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_type.hpp"
#include "system/io/stream.hpp"

class IContentProcessor
{
template <typename TFriendArg0>
friend class IContentProcessor_1;
public:
    virtual Type* get_OutputType() = 0;

    virtual void* ReadObject(::Stream* stream) = 0;
};
