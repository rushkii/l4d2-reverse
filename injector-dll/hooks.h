#pragma once
#include <d3d9.h>

namespace hooks {
	void Initialize();
	void Shutdown();

	using EndScene_t = HRESULT(WINAPI*)(IDirect3DDevice9*);
	extern EndScene_t oEndScene;

	using WndProc_t = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);
	extern WndProc_t oWndProc;

	using Reset_t = HRESULT(WINAPI *)(IDirect3DDevice9 *, D3DPRESENT_PARAMETERS *);
	extern Reset_t oReset;
    

	HRESULT WINAPI hkEndScene(IDirect3DDevice9* pDevice);
	LRESULT CALLBACK hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HRESULT WINAPI hkReset(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters);
}