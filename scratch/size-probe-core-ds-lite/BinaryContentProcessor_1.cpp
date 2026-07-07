#ifdef DrawText
#undef DrawText
#endif
#include "BinaryContentProcessor_1.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_type.hpp"
#include "system/func.hpp"
#include "system/io/stream.hpp"
#include "BinaryContentProcessor_1.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_type.hpp"
#include "system/io/stream.hpp"

template <typename T>
Type* BinaryContentProcessor_1<T>::get_OutputType()
{
return he_cpp_type_of<T>("T");
}

template <typename T>
BinaryContentProcessor_1<T>::BinaryContentProcessor_1(Func<Stream*, T>* reader) : Reader()
{
this->Reader = (reader != nullptr ? reader : throw new ArgumentNullException("reader"));
}

template <typename T>
T BinaryContentProcessor_1<T>::Read(::Stream* stream)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
return (*this->Reader)(stream);}

template <typename T>
void* BinaryContentProcessor_1<T>::ReadObject(::Stream* stream)
{
return this->Read(stream);}

