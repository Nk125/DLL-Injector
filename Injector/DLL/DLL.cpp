#include <CONFIG.h>
#include <windows.h>
#include <psapi.h>
#include <string>
#include <tchar.h>
#include MAINPROCPATH

#if !AWAIT
bool kill_main_proc() {
    TCHAR mod[MAX_PATH]; // Parent Exe Module Filename
    DWORD l = GetModuleFileName(NULL, mod, sizeof(mod) / sizeof(TCHAR));

    if (l <= 0) {
        return true;
    }

    DWORD aProcesses[1024], cbNeeded, cProcesses;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        return false;
    }

    cProcesses = cbNeeded / sizeof(DWORD);

    for (DWORD i = 0; i < cProcesses; i++) {
        if (aProcesses[i] != 0) {
            TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE, aProcesses[i]);

            if (NULL != hProcess) {
                HMODULE hMod;
                DWORD cbNeeded;

                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
                    GetModuleFileName(hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
                    
#ifdef UNICODE
                    if (std::wstring{ mod } == std::wstring{ szProcessName }) {
#else
                    if (std::string{ mod } == std::string{ szProcessName }) {
#endif
                        TerminateProcess(hProcess, 0);
                    }
                }
            }

            CloseHandle(hProcess);
        }
    }

    return true;
}
#endif

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        try {
            MAIN(hModule);
        }
        catch (...) {
            return false; // This will free the DLL
        }

#if !AWAIT
        return kill_main_proc(); // After the main procedure is called, the injected process is killed
#endif
    }

    return true;
}
