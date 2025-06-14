#include "pch.h"
#include "globals.h"

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

    char tempBuffer[64] = { 0 };
    SIZE_T bytesRead;

    if(!ReadProcessMemory(hProcess, (LPCVOID)address, tempBuffer, sizeof(tempBuffer) - 1, &bytesRead)) {
        strncpy_s(buffer, bufferSize, "N/A", bufferSize - 1);
        return false;
    }

    tempBuffer[bytesRead] = '\0';

    if(bytesRead > 0 && tempBuffer[0] != '\0' && strlen(tempBuffer) > 0) {
        strncpy_s(buffer, bufferSize, tempBuffer, bufferSize - 1);
        return true;
    } else {
        strncpy_s(buffer, bufferSize, "N/A", bufferSize - 1);
        return false;
    }
}
