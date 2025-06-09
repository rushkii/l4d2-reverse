#include "pch.h"
#include "globals.h"
#include "util.h"
#include "mem.h"


HMODULE g_hModule = NULL;
bool g_bShutdownSignal = false;
HANDLE g_hConsoleOutput = NULL;

// Thread to handle unloading the DLL
DWORD WINAPI UnloadRoutine(LPVOID lpParam) {
    if(g_hConsoleOutput != NULL) {
        COORD finalCursorPos = { 0, 7 };
        SetConsoleCursorPosition(g_hConsoleOutput, finalCursorPos);
        std::cout << "Unloading DLL and closing console..." << std::endl;
        Sleep(1500);
        FreeConsole();
        g_hConsoleOutput = NULL;
    }
    FreeLibraryAndExitThread(g_hModule, 0);
    return 0;
}


DWORD WINAPI HackThread(HMODULE hModule) {
    std::string text = "[HackThread] HackThread started.\n";

    if(g_hConsoleOutput) {
        SetConsoleCursorPosition(g_hConsoleOutput, { 0, 2 });
        text += "";
        std::cout << "[HackThread] Initializing..." << std::endl;
    }

    if(g_hConsoleOutput) {
        std::cout << "[HackThread] Hello from the DLL!" << std::endl;
    }

    DWORD pid = GetCurrentProcessId();
    HANDLE hProcess = GetCurrentProcess();

    if(g_hConsoleOutput) {
        std::cout << "[HackThread] Current PID: " << pid << std::endl;
    }

    COORD ammoDisplayPos = { 0, 13 };
    COORD healthDisplayPos = { 0, 14 };

    //const wchar_t *moduleName = L"server.dll";
    const wchar_t *moduleName = L"engine.dll";

    uintptr_t moduleBase = GetModuleBaseAddress(pid, moduleName);

    if(g_hConsoleOutput) {
        std::cout << "[HackThread] Loading module..." << std::endl;
    }

    while(!moduleBase) {
        Sleep(1000);
        moduleBase = GetModuleBaseAddress(pid, moduleName);
    }

    if(g_hConsoleOutput) {
        std::cout << "[HackThread] Module loaded!" << std::endl;
    }

    if(g_hConsoleOutput) {
        std::wcout << L"[HackThread] " << moduleName << L" base: 0x" << std::hex << moduleBase << std::dec << std::endl;
    }

    // Regular Pistol
    /*uintptr_t pistolAmmoPtr = moduleBase + 0x007D6D3C;
    std::vector<unsigned int> ammoOffsets = { 0x0, 0x60, 0x164, 0x1414 };*/

    //Health (Readable/Writable)
    //uintptr_t healthPointer = moduleBase + 0x0075E9C8;
    //std::vector<unsigned int> healthOffsets = { 0x0, 0x60, 0x8, 0xC, 0xEC };

    // Health (UI?)
    //uintptr_t originalHpPtr = moduleBase + 0x0075775C;
    //std::vector<unsigned int> healthOffsets = { 0x44, 0x40, 0x80, 0x4, 0x8, 0x24, 0x1B8 };

    // Health (engine.dll highly readable/writable)
    uintptr_t originalHpPtr = moduleBase + 0x00429310;
    std::vector<unsigned int> healthOffsets = { 0x8, 0x44, 0x8, 0x18, 0x1EC, 0xEC };

    // Map ID (this address could be wrong, I will update it later.
    //uintptr_t mapIdPtr = moduleBase + 0x0003D870;
    //std::vector<unsigned int> mapIdOffsets = { 0x47C, 0x8, 0x2F8, 0x80, 0x8, 0x154 };

    const unsigned int newValue = 90;

    // Main loop for real-time updates
    while(!g_bShutdownSignal) {
        // Check for shutdown hotkey (e.g., END key)
        if(GetAsyncKeyState(VK_END) & 0x8000) {
            if(g_hConsoleOutput) {
                SetConsoleCursorPosition(g_hConsoleOutput, ammoDisplayPos);
                std::cout << "                                        ";
                SetConsoleCursorPosition(g_hConsoleOutput, ammoDisplayPos);
                std::cout << "[HackThread] END key pressed. Shutting down..." << std::endl;
            }
            g_bShutdownSignal = true;
            break;
        }

        //uintptr_t pistolAmmoAddr = FindDMAAddy(hProcess, pistolAmmoPtr, ammoOffsets);
        uintptr_t originalHpAddr = FindDMAAddy(hProcess, originalHpPtr, healthOffsets);
        //uintptr_t mapIdAddr = FindDMAAddy(hProcess, mapIdPtr, mapIdOffsets);

        // hotkey F3: change HP value.
        if(originalHpAddr && GetAsyncKeyState(VK_F3)) {
            BOOL success = WriteProcessMemory(hProcess, (LPVOID)originalHpAddr, &newValue, sizeof(newValue), NULL);
            if(!success) {
                if(g_hConsoleOutput) {
                    std::cerr << "Error writing memory: " << GetLastError() << std::endl;
                }
            }
        }

        //int ammoValue = 0;
        int healthValue = 0;
        //char mapIdBuffer[64] = { 0 };

        /*if(pistolAmmoAddr != 0) {
            if(ReadMemory(pistolAmmoAddr, ammoValue)) {
                if(g_hConsoleOutput) {
                    SetConsoleCursorPosition(g_hConsoleOutput, ammoDisplayPos);
                    std::cout << "[HackThread] Current Ammo: " << ammoValue << "         \n\n\n";
                }
            }
        }*/

        if(originalHpAddr != 0) {
            if(ReadMemoryInt(originalHpAddr, healthValue)) {
                if(g_hConsoleOutput) {
                    SetConsoleCursorPosition(g_hConsoleOutput, healthDisplayPos);
                    std::cout << "[HackThread] Current Health: " << healthValue << "         \n\n\n";
                }
            }
        }

        //if(mapIdAddr != 0) {
        //    if(ReadMemoryStr(hProcess, mapIdAddr, mapIdBuffer, sizeof(mapIdBuffer))) {
        //        if(g_hConsoleOutput) {
        //            SetConsoleCursorPosition(g_hConsoleOutput, ammoDisplayPos);
        //            std::cout << "[HackThread] Current Map: " << mapIdBuffer << "                    \n\n\n";
        //        }
        //    }
        //}
        Sleep(100);
    }

    HANDLE hUnloader = CreateThread(NULL, 0, UnloadRoutine, NULL, 0, NULL);
    if(hUnloader) {
        CloseHandle(hUnloader);
    } else if(g_hConsoleOutput) {
        SetConsoleCursorPosition(g_hConsoleOutput, { static_cast<SHORT>(0), static_cast<SHORT>(ammoDisplayPos.Y + 2) });
        std::cout << "[HackThread] ERROR: Could not create unload thread!" << std::endl;
    }

    if(g_hConsoleOutput) {
        std::cout << "[HackThread] HackThread finished." << std::endl;
    }

    return 0;
}


BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {
    HANDLE hThread = NULL;
    FILE *pFile_stdout = nullptr;
    FILE *pFile_stdin = nullptr;

    switch(ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        g_hModule = hModule;
        g_bShutdownSignal = false;

        if(AllocConsole()) {
            if(freopen_s(&pFile_stdout, "CONOUT$", "w", stdout) == 0) {
                setvbuf(stdout, NULL, _IONBF, 0);
                g_hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
                if(g_hConsoleOutput != NULL && g_hConsoleOutput != INVALID_HANDLE_VALUE) {
                    std::cout << "DLL Attached. Console Initialized." << std::endl;
                    std::cout << "---------------------------------" << std::endl;
                }
            } else {
                OutputDebugStringA("Failed to redirect stdout to console.\n");
            }

        } else {
            OutputDebugStringA("AllocConsole failed. GetLastError() might provide info.\n");
        }

        DisableThreadLibraryCalls(hModule);
        hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, NULL);
        if(hThread) {
            CloseHandle(hThread);
        } else if(g_hConsoleOutput) {
            std::cout << "FATAL ERROR: Failed to create HackThread!" << std::endl;
            OutputDebugStringA("Failed to create HackThread.\n");
            FreeConsole();
            g_hConsoleOutput = NULL;
            return FALSE;
        }
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        g_bShutdownSignal = true;

        // If lpReserved == NULL, DLL is being unloaded via FreeLibrary (likely by our UnloadRoutine)
        // If lpReserved != NULL, the process is terminating.
        // UnloadRoutine should handle FreeConsole if triggered by hotkey.
        // If process is terminating, OS handles console cleanup.
        OutputDebugStringA("DLL_PROCESS_DETACH triggered.\n");
        if(lpReserved == NULL) {
            OutputDebugStringA("DLL unloaded via FreeLibrary.\n");
        } else {
            OutputDebugStringA("DLL detaching because process is terminating.\n");
        }
        g_hConsoleOutput = NULL;

        FreeConsole();
        break;
    }
    return TRUE;
}
