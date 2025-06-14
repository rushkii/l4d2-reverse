#include <Windows.h>
#include <TlHelp32.h>
#include <string.h>
#include <wchar.h>
#include <tchar.h>
#include <iostream>

DWORD FindProcessIdByName(LPCTSTR processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnapshot == INVALID_HANDLE_VALUE) {
        return -1;
    }
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if(!Process32First(hSnapshot, &pe)) {
        CloseHandle(hSnapshot);
        return -1;
    }
    do {
        if(_tcscmp(pe.szExeFile, processName) == 0) {
            CloseHandle(hSnapshot);
            return pe.th32ProcessID;
        }
    } while(Process32Next(hSnapshot, &pe));
    CloseHandle(hSnapshot);
    return -1;
}

char *GetFileFullPath(const char *dllName) {
    static char dllFullPath[MAX_PATH];
    if(GetFullPathNameA(dllName, MAX_PATH, dllFullPath, NULL) == 0) {
        return NULL;
    }
    return dllFullPath;
}

void print(const char *msg) {
    std::cout << msg << std::endl;
}