#include "pch.h"
#include "globals.h"
#include "util.h"
#include "mem.h"
#include "hooks.h"

HMODULE                 g_hModule = NULL;
bool                    g_bShutdownSignal = false;
bool                    g_bShowMenu = true;
int                     g_healthValue = 0;
char                    g_mapIdBuffer[64] = "N/A";

DWORD WINAPI UnloadRoutine(LPVOID lpParam) {
    g_bShutdownSignal = true;
    Sleep(1000);
    hooks::Shutdown();
    Sleep(500);
    FreeLibraryAndExitThread(g_hModule, 0);
    return 0;
}

void CloseHandleDLL() {
    HANDLE hUnloader = CreateThread(NULL, 0, UnloadRoutine, NULL, 0, NULL);
    if(hUnloader) {
        CloseHandle(hUnloader);
    }
}

DWORD WINAPI HackThread(HMODULE hModule) {
    try {
        hooks::Initialize();
    } catch(...) {
        CloseHandleDLL();
        return 0;
    }

    DWORD pid = GetCurrentProcessId();
    HANDLE hProcess = GetCurrentProcess();

    const wchar_t *moduleName = L"engine.dll";
    uintptr_t moduleBase = GetModuleBaseAddress(pid, moduleName);

    while(!moduleBase) {
        Sleep(1000);
        moduleBase = GetModuleBaseAddress(pid, moduleName);
    }

    // Regular Pistol
    //uintptr_t pistolAmmoPtr = moduleBase + 0x007D6D3C;
    //std::vector<unsigned int> ammoOffsets = { 0x0, 0x60, 0x164, 0x1414 };

    // Health
    uintptr_t originalHpPtr = moduleBase + 0x00429310;
    std::vector<unsigned int> healthOffsets = { 0x8, 0x44, 0x8, 0x18, 0x1EC, 0xEC };
    const unsigned int newHealthValue = 100;

    // Map ID
    uintptr_t mapIdPtr = moduleBase + 0x000030B4;
    std::vector<unsigned int> mapIdOffsets = { 0x10 };

    // Main loop for real-time updates
    while(!g_bShutdownSignal) {
        // Check for shutdown hotkey (e.g., END key)
        if(GetAsyncKeyState(VK_END) & 0x8000) {
            g_bShutdownSignal = true;
            break;
        }

        //uintptr_t pistolAmmoAddr = FindDMAAddy(hProcess, pistolAmmoPtr, ammoOffsets);
        uintptr_t originalHpAddr = FindDMAAddy(hProcess, originalHpPtr, healthOffsets);
        uintptr_t mapIdAddr = FindDMAAddy(hProcess, mapIdPtr, mapIdOffsets);

        if(originalHpAddr && (GetAsyncKeyState(VK_F3) & 0x8000)) {
            WriteProcessMemory(hProcess, (LPVOID)originalHpAddr, &newHealthValue, sizeof(newHealthValue), NULL);
        }

        if(!ReadMemoryInt(originalHpAddr, g_healthValue)) {
            g_healthValue = 0;
        }

        ReadMemoryStr(hProcess, mapIdAddr, g_mapIdBuffer, sizeof(g_mapIdBuffer));

        Sleep(50);
    }

    CloseHandleDLL();
    return 0;
}


BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {
    HANDLE hThread = NULL;

    switch(ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        g_hModule = hModule;
        g_bShutdownSignal = false;

        DisableThreadLibraryCalls(hModule);
        hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, NULL);
        if(hThread) {
            CloseHandle(hThread);
        } else {
            return FALSE;
        }
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        g_bShutdownSignal = true;
        Sleep(1000);
        break;
    }
    return TRUE;
}
