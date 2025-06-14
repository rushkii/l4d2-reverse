#pragma once
#include "globals.h"

uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets);
bool ReadMemoryInt(uintptr_t address, int &outValue);
bool ReadMemoryStr(HANDLE hProcess, uintptr_t address, char *buffer, size_t bufferSize);