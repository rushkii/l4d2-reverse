#include "pch.h"
#include "hooks.h"
#include "globals.h"
#include "MinHook.h"
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx9.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

hooks::EndScene_t hooks::oEndScene = nullptr;
hooks::Reset_t hooks::oReset = nullptr;
hooks::WndProc_t hooks::oWndProc = nullptr;

static HWND g_window = NULL;
static bool g_isImGuiInitialized = false;

void RenderUI() {
    if(!g_bShowMenu) {
        return;
    }

    ImGui::Begin("Left 4 Dead 2", &g_bShowMenu);

    ImGui::Text("Map: %s", g_mapIdBuffer);
    ImGui::Text("HP: %d (%s)", g_healthValue, "Alive");
    ImGui::Text("Status: %s", !g_bShutdownSignal ? "Ready" : "Shutting Down...");
    ImGui::Separator();
    ImGui::Text("Press F3 to full HP.");
    ImGui::Text("Press INSERT to hide menu.");
    ImGui::Text("Press END to unload DLL.");

    ImGui::End();
}

float CalcFontSize(float height) {
    float font_size = height * 0.01f;
    return max(12.0f, font_size);
}

HRESULT WINAPI hooks::hkReset(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters) {
    if(g_isImGuiInitialized) {
        ImGui_ImplDX9_InvalidateDeviceObjects();
    }

    HRESULT hr = oReset(pDevice, pPresentationParameters);

    if(SUCCEEDED(hr)) {
        if(g_isImGuiInitialized) {
            ImGuiIO &io = ImGui::GetIO();
            io.Fonts->Clear();

            float new_height = static_cast<float>(pPresentationParameters->BackBufferHeight);

            ImFontConfig font_config;
            font_config.SizePixels = CalcFontSize(new_height);
            io.Fonts->AddFontDefault(&font_config);

            ImGui_ImplDX9_CreateDeviceObjects();
        }
    }

    return hr;
}

HRESULT WINAPI hooks::hkEndScene(IDirect3DDevice9 *pDevice) {
    if(!g_isImGuiInitialized && pDevice) {
        D3DDEVICE_CREATION_PARAMETERS params;
        if(SUCCEEDED(pDevice->GetCreationParameters(&params))) {
            g_window = params.hFocusWindow;
            if(g_window) {
                oWndProc = (WNDPROC)SetWindowLongPtr(g_window, GWLP_WNDPROC, (LONG_PTR)hkWndProc);

                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO &io = ImGui::GetIO();
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
                io.IniFilename = NULL;
                ImGui::StyleColorsDark();

                RECT client_rect;
                if(GetClientRect(g_window, &client_rect)) {
                    float window_height = static_cast<float>(client_rect.bottom - client_rect.top);
                    
                    ImFontConfig font_config;
                    font_config.SizePixels = CalcFontSize(window_height);
                    io.Fonts->AddFontDefault(&font_config);
                }

                ImGui_ImplWin32_Init(g_window);
                ImGui_ImplDX9_Init(pDevice);

                void **pVTable = *reinterpret_cast<void ***>(pDevice);
                if(MH_CreateHook(pVTable[16], &hkReset, reinterpret_cast<void **>(&oReset)) == MH_OK) {
                    MH_EnableHook(pVTable[16]);
                }

                g_isImGuiInitialized = true;
            }
        }
    }

    if(g_isImGuiInitialized) {
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        RenderUI();
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    return oEndScene(pDevice);
}

LRESULT CALLBACK hooks::hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if(uMsg == WM_KEYUP && wParam == VK_INSERT) {
        g_bShowMenu = !g_bShowMenu;
    }
    if(g_isImGuiInitialized && g_bShowMenu && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
        return true;
    }
    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

void hooks::Initialize() {
    if(MH_Initialize() != MH_OK) return;

    IDirect3D9 *pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if(!pD3D) return;

    HWND tempWindow = CreateWindowA("BUTTON", "Temp", WS_SYSMENU, 0, 0, 1, 1, NULL, NULL, GetModuleHandle(NULL), NULL);
    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = tempWindow;

    IDirect3DDevice9 *pDevice = nullptr;
    if(SUCCEEDED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, tempWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice))) {
        void **pVTable = *reinterpret_cast<void ***>(pDevice);
        if(MH_CreateHook(pVTable[42], &hkEndScene, reinterpret_cast<void **>(&oEndScene)) == MH_OK) {
            MH_EnableHook(MH_ALL_HOOKS);
        }
        pDevice->Release();
    }
    pD3D->Release();
    DestroyWindow(tempWindow);
}

void hooks::Shutdown() {
    if(oWndProc && g_window) {
        SetWindowLongPtr(g_window, GWLP_WNDPROC, (LONG_PTR)oWndProc);
    }

    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    if(g_isImGuiInitialized) {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    g_isImGuiInitialized = false;
}
