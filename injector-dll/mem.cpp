#include "pch.h"
#include <vector>

uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets) {
    uintptr_t addr = ptr;
    for(unsigned int i = 0; i < offsets.size(); ++i) {
        ReadProcessMemory(hProc, (BYTE *)addr, &addr, sizeof(addr), 0);
        addr += offsets[i];
    }
    return addr;
}

bool ReadMemoryInt(uintptr_t address, int &outValue) {
    __try {
        outValue = *(int *)address;
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool ReadMemoryStr(HANDLE hProcess, uintptr_t address, char *buffer, size_t bufferSize) {
    if(buffer == nullptr || bufferSize == 0) {
        return false;
    }

    // Read bytes from the process until the buffer is almost full (leaving space for null terminator)
    // or until a null terminator is found.
    // For simplicity, we'll read up to bufferSize-1 and then null-terminate.
    // Realistically, you might read byte by byte until a null is hit, but this is simpler.
    SIZE_T bytesRead;
    if(!ReadProcessMemory(hProcess, (LPCVOID)address, buffer, bufferSize - 1, &bytesRead)) {
        buffer[0] = '\0'; // Ensure it's an empty string on failure
        return false;
    }

    // Always null-terminate the buffer after reading
    buffer[bytesRead] = '\0';
    return true;
}