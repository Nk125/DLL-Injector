#include <windows.h>

void MAIN(HINSTANCE hModule) {
	typedef int (WINAPI* mboxptr)(HWND, LPCSTR, LPCSTR, UINT);

	mboxptr msgbox = (mboxptr)GetProcAddress(LoadLibraryA("user32.dll"), "MessageBoxA");
	msgbox(NULL, "If this is shown, the DLL was injected succesfully!", "DLL Message Box", NULL);
	return;
}