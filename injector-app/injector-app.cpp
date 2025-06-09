#include <iostream>
#include <string>
#include <Windows.h>
#include <TlHelp32.h>
#include "util.h"
#include <conio.h>


int main() {
    const wchar_t *appName = L"left4dead2.exe";
    const char *DLL_PATH = GetFileFullPath("injector-dll.dll");
    size_t dllSize = strlen(DLL_PATH) + 1;

    DWORD pid = FindProcessIdByName(appName);
    if(!pid) {
        std::wcout << L"Process not found: " << appName << std::endl;
        _getch();
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if(!hProcess) {
        std::wcout << L"Failed to open process: " << appName << std::endl;
        _getch();
        return 1;
    }

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

    WaitForSingleObject(hThread, INFINITE);
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
