#include <cstdint>

extern "C" {
#include <nds/interrupts.h>
}

/// <summary>
/// Executes one atomic fallback operation while interrupts are masked so the Nintendo DS runtime observes a stable critical section.
/// </summary>
/// <typeparam name="TResult">Return type of the atomic operation.</typeparam>
/// <param name="callback">Operation to execute inside the critical section.</param>
/// <returns>The callback result.</returns>
template <typename TResult, typename TCallback>
TResult RunAtomicCriticalSection(TCallback callback) {
    int previousInterruptState = enterCriticalSection();
    TResult result = callback();
    leaveCriticalSection(previousInterruptState);
    return result;
}

/// Provides the 64-bit fetch-add helper expected by generated core when the ARM toolchain does not ship libatomic.
extern "C" unsigned long long __atomic_fetch_add_8(volatile void* pointer, unsigned long long value, int memoryOrder) {
    (void)memoryOrder;

    return RunAtomicCriticalSection<unsigned long long>([&]() {
        volatile unsigned long long* typedPointer = static_cast<volatile unsigned long long*>(pointer);
        unsigned long long previousValue = *typedPointer;
        *typedPointer = previousValue + value;
        return previousValue;
    });
}

/// Provides the 32-bit fetch-add helper expected by generated core when the ARM toolchain does not ship libatomic.
extern "C" unsigned int __atomic_fetch_add_4(volatile void* pointer, unsigned int value, int memoryOrder) {
    (void)memoryOrder;

    return RunAtomicCriticalSection<unsigned int>([&]() {
        volatile unsigned int* typedPointer = static_cast<volatile unsigned int*>(pointer);
        unsigned int previousValue = *typedPointer;
        *typedPointer = previousValue + value;
        return previousValue;
    });
}

/// Provides the 32-bit fetch-sub helper expected by generated core when the ARM toolchain does not ship libatomic.
extern "C" unsigned int __atomic_fetch_sub_4(volatile void* pointer, unsigned int value, int memoryOrder) {
    (void)memoryOrder;

    return RunAtomicCriticalSection<unsigned int>([&]() {
        volatile unsigned int* typedPointer = static_cast<volatile unsigned int*>(pointer);
        unsigned int previousValue = *typedPointer;
        *typedPointer = previousValue - value;
        return previousValue;
    });
}

/// Provides the 1-byte exchange helper expected by generated core when the ARM toolchain does not ship libatomic.
extern "C" unsigned char __atomic_exchange_1(volatile void* pointer, unsigned char value, int memoryOrder) {
    (void)memoryOrder;

    return RunAtomicCriticalSection<unsigned char>([&]() {
        volatile unsigned char* typedPointer = static_cast<volatile unsigned char*>(pointer);
        unsigned char previousValue = *typedPointer;
        *typedPointer = value;
        return previousValue;
    });
}

/// Provides the 32-bit compare-exchange helper expected by generated core when the ARM toolchain does not ship libatomic.
extern "C" bool __atomic_compare_exchange_4(
    volatile void* pointer,
    void* expected,
    unsigned int desired,
    bool weak,
    int successMemoryOrder,
    int failureMemoryOrder) {
    (void)weak;
    (void)successMemoryOrder;
    (void)failureMemoryOrder;

    return RunAtomicCriticalSection<bool>([&]() {
        volatile unsigned int* typedPointer = static_cast<volatile unsigned int*>(pointer);
        unsigned int* expectedPointer = static_cast<unsigned int*>(expected);
        unsigned int currentValue = *typedPointer;
        if (currentValue == *expectedPointer) {
            *typedPointer = desired;
            return true;
        }

        *expectedPointer = currentValue;
        return false;
    });
}
