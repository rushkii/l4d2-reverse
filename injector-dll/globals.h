#pragma once
#include <vector>
#include <string>
#include <mutex>
#include <atomic>
#include <Windows.h>
#include <cstdio>
#include <TlHelp32.h>
#include <iostream>
#include <wchar.h>
#include <tchar.h>
#include <cstdint>


// DLL module handle, needed for unloading
extern HMODULE g_hModule;

// The master switch to signal all loops to exit
extern bool g_bShutdownSignal;

// Controls the visibility of the ImGui menu
extern bool g_bShowMenu;

// The current health value read from the game
extern int g_healthValue;

// The current map ID string read from the game
extern char g_mapIdBuffer[64];
