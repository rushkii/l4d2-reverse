#include <iostream>
#include <string>
#include <Windows.h>
#include <TlHelp32.h>
#include "util.h"
#include <conio.h>

int main() {
    const wchar_t *appName = L"left4dead2.exe";
    const char *dllName = "injector-dll.dll";

    size_t dllSize = 0;
    HANDLE hProcess = 0;

    const char *DLL_PATH = GetFileFullPath(dllName);
    if(DLL_PATH == nullptr || strlen(DLL_PATH) == 0) {
        std::cout << "Error: Could not find injector-dll.dll. Make sure it's in the same folder as the injector-app.exe." << std::endl;
        _getch();
        return 1;
    }

    dllSize = strlen(DLL_PATH) + 1;

    DWORD pid = FindProcessIdByName(appName);
    if(!pid) {
        std::wcout << appName << " not found, does it exist?" << std::endl;
        _getch();
        return 1;
    }

    std::wcout << "Waiting for " << appName << " to open..." << std::endl;
    while(!hProcess) {
        pid = FindProcessIdByName(appName);
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    }
    std::wcout << appName << L" has been opened." << std::endl;

    LPVOID lpBaseAddress = VirtualAllocEx(hProcess, NULL, dllSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if(!lpBaseAddress) {
        print("Failed to allocate memory in target process!");
        CloseHandle(hProcess);
        _getch();
        return 1;
    }
    std::cout << "Memory allocated at: 0x" << std::hex << (uintptr_t)lpBaseAddress << std::dec << std::endl;

    int memoryValue = WriteProcessMemory(hProcess, lpBaseAddress, DLL_PATH, dllSize, NULL);
    if(!memoryValue) {
        print("Failed to write memory in target process!");
        VirtualFreeEx(hProcess, lpBaseAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        _getch();
        return 1;
    }
    std::cout << "DLL path written to target process." << std::endl;

    HMODULE hk32 = GetModuleHandleA("kernel32.dll");
    if(!hk32) {
        print("Failed to get handle to Kernel32.dll!");
        VirtualFreeEx(hProcess, lpBaseAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        _getch();
        return 1;
    }

    FARPROC loadLibraryAddr = GetProcAddress(hk32, "LoadLibraryA");
    if(!loadLibraryAddr) {
        print("Failed to get address of LoadLibraryA!");
        VirtualFreeEx(hProcess, lpBaseAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        _getch();
        return 1;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, lpBaseAddress, 0, NULL);
    if(!hThread) {
        print("Failed to create remote thread!");
        VirtualFreeEx(hProcess, lpBaseAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        _getch();
        return 1;
    }

    WaitForSingleObject(hThread, 5000);
    DWORD threadExitCode = 0;
    GetExitCodeThread(hThread, &threadExitCode);

    CloseHandle(hThread);
    VirtualFreeEx(hProcess, lpBaseAddress, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    if(threadExitCode == 0) {
        print("LoadLibraryA failed in the remote process! DLL not injected.");
        _getch();
        return 1;
    }

    std::cout << "DLL injected successfully! Module handle: 0x" << std::hex << threadExitCode << std::dec << std::endl;
    _getch();
    return 0;
}
