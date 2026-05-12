#include <cstdint>

extern "C" {
#include <nds/interrupts.h>
}

/// Provides the 64-bit fetch-add helper expected by generated core when the ARM toolchain does not ship libatomic.
extern "C" unsigned long long __atomic_fetch_add_8(volatile void* pointer, unsigned long long value, int memoryOrder) {
    (void)memoryOrder;

    int previousInterruptState = enterCriticalSection();
    volatile unsigned long long* typedPointer = static_cast<volatile unsigned long long*>(pointer);
    unsigned long long previousValue = *typedPointer;
    *typedPointer = previousValue + value;
    leaveCriticalSection(previousInterruptState);
    return previousValue;
}
