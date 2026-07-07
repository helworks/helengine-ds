#ifdef DrawText
#undef DrawText
#endif
#include "NativeOwnership.hpp"
#include "runtime/array.hpp"
#include "NativeOwnership.hpp"
#include "runtime/native_disposable.hpp"
#include "runtime/array.hpp"
#include "runtime/native_disposable.hpp"

template <typename T>
void NativeOwnership::Delete(T value)
{
    if (value != nullptr)
    {
        delete value;
    }
}

template <typename T>
void NativeOwnership::DeleteItemsAndRelease__ref0(Array<T>*& values)
{
    if (values != nullptr)
    {
for (int32_t index = 0; index < values->get_Length(); index++) {
delete (*values)[index];
}
    }
delete values;
values = nullptr;
}

template <typename T>
void NativeOwnership::DisposeAndDelete(T value)
{
    if (value != nullptr)
    {
        value->Dispose();
        delete value;
    }
}

template <typename T>
void NativeOwnership::DisposeAndRelease__ref0(T& value)
{
    if (value != nullptr)
    {
        value->Dispose();
        delete value;
    }
    value = nullptr;
}

template <typename T>
void NativeOwnership::DisposeItemsAndRelease__ref0(Array<T>*& values)
{
    if (values != nullptr)
    {
for (int32_t index = 0; index < values->get_Length(); index++) {
if ((*values)[index] != nullptr)
{
(*values)[index]->Dispose();
delete (*values)[index];
}
}
    }
delete values;
values = nullptr;
}

template <typename T>
void NativeOwnership::Release__ref0(T& value)
{
    if (value != nullptr)
    {
        delete value;
    }
    value = nullptr;
}

