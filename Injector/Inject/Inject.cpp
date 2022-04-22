#include <CONFIG.h>
#ifdef _DEBUG
#include <iostream>
#endif
#include <string>
#include <windows.h>

DWORD pidproc;

void exitproc(int ret) {
    HANDLE proc = OpenProcess(PROCESS_TERMINATE, 0, pidproc);

    if (proc != INVALID_HANDLE_VALUE) {
        TerminateProcess(proc, 0);
    }

    std::exit(ret);
}

#ifdef _DEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#endif
    std::string exe(EXEPATH), cdir(CDIR), dll(DLLPATH);
    std::string params(PARAMS);

    /*
     Exe:
      Full path to executable to run
     Params:
      Arguments to pass to the executable
     Cdir:
      Current Directory to pass to the process
     Dll:
      The DLL path/name to inject
    */

#ifdef _DEBUG
    std::cout << "Opening executable\n";
#endif

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    STARTUPINFOA st;
    ZeroMemory(&st, sizeof(st));
    st.wShowWindow = SW_HIDE;
    st.cb = sizeof(st);
#ifdef _DEBUG
    std::cout << "Startup Info Set!\n";
#endif

    if (!CreateProcessA(exe.c_str(), &params[0], NULL, NULL, 0, DETACHED_PROCESS | CREATE_SUSPENDED, NULL, cdir.c_str(), &st, &pi)) {
#ifdef _DEBUG
        std::cout << "Error opening exe: " << GetLastError() << "\n";
#endif
        return 1;
    }
#ifdef _DEBUG
    else {
        std::cout << "Executable started with PID: " << pi.dwProcessId << "\n";
    }
#endif

#if POPEN
    DWORD perms = PROCESS_CREATE_THREAD | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_TERMINATE;
    HANDLE proc = OpenProcess(perms, FALSE, pi.dwProcessId);
#else
    HANDLE proc = pi.hProcess;
    pidproc = pi.dwProcessId;
#endif
    HANDLE tr;
    std::string szLibPath;
    void* pLibRemote;
#if AWAIT
    DWORD hLibModule;
#endif
    HMODULE hKernel32 = GetModuleHandleA("Kernel32");

    szLibPath.resize(MAX_PATH);
#ifdef _DEBUG
    std::cout << "Loading DLL\n";
#endif

    if (GetModuleFileNameA(LoadLibraryExA(dll.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES), &szLibPath[0], MAX_PATH) == 0) {
#ifdef _DEBUG
        std::cout << "DLL couldn't be loaded\n";
#endif
        exitproc(2);
    }

#ifdef _DEBUG
    std::cout << "DLL Loaded with path: " << szLibPath << "\n";
#endif

    pLibRemote = VirtualAllocEx(proc, NULL, szLibPath.size(), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

#ifdef _DEBUG
    std::cout << "Memory allocated with VirtualAlloc\n";
#endif

    if (!WriteProcessMemory(proc, pLibRemote, (void*)szLibPath.data(), szLibPath.size(), NULL)) {
#ifdef _DEBUG
        std::cout << "Error at writing memory process: " << GetLastError() << "\n";
#endif
        exitproc(3);
    }
#ifdef _DEBUG
    else {
        std::cout << "Memory Process written\n\n";
    }
#endif

    tr = CreateRemoteThread(proc, NULL, 0, (LPTHREAD_START_ROUTINE) GetProcAddress(hKernel32, "LoadLibraryA"), pLibRemote, 0, NULL);

#ifdef _DEBUG
    std::cout << "Remote thread created\n";
#endif

#if AWAIT
#ifdef _DEBUG
    std::cout << "Waiting remote thread to finish...\n";
#endif
    WaitForSingleObject(tr, INFINITE);

#ifdef _DEBUG
    std::cout << "Remote thread finished\n\n";
#endif

    if (!GetExitCodeThread(tr, &hLibModule)) {
#ifdef _DEBUG
        std::cout << "Error at getting exit code: " << GetLastError() << "\n";
#endif
        exitproc(4);
    }
#ifdef _DEBUG
    else {
        std::cout << "Got exit code (LoadLibrary)\n";
    }

    if (hLibModule == STATUS_PENDING) {
        std::cout << "LoadLibrary thread still running!\n\n";
    }
    else {
        std::cout << "Return code: " << hLibModule << "\n\n"; // I think the return code is the memory address of the DLL
    }
#endif

#ifdef _DEBUG
    std::cout << "Freeing DLL from executable\n";
#endif

    tr = CreateRemoteThread(proc, NULL, 0, (LPTHREAD_START_ROUTINE) GetProcAddress(hKernel32, "FreeLibrary"), pLibRemote, 0, NULL);
    WaitForSingleObject(tr, INFINITE);

#ifdef _DEBUG
    std::cout << "FreeLibrary remote thread finished\n";
#endif

    if (!GetExitCodeThread(tr, &hLibModule)) {
#ifdef _DEBUG
        std::cout << "Error at getting exit code, the process was killed by the DLL?\n";
#endif
    }
#ifdef _DEBUG
    else {
        std::cout << "Got exit code (FreeLibrary)\n";
    }

    if (hLibModule == STATUS_PENDING) {
        std::cout << "FreeLibrary thread still running!\n\n";
    }
    else {
        std::cout << "Return code: " << hLibModule << "\n\n";
    }
#endif

    if (!VirtualFreeEx(proc, pLibRemote, szLibPath.size(), MEM_DECOMMIT)) {
#ifdef _DEBUG
        std::cout << "Error at freeding virtual mem: " << GetLastError() << "\n";
#endif
        exitproc(5);
    }
#ifdef _DEBUG
    else {
        std::cout << "Virtual memory freed\n";
    }
#endif

    TerminateProcess(proc, 0); // The DLL don't kill the process when you await
#endif

    // Cleanup
    CloseHandle(tr);
    CloseHandle(proc);
    CloseHandle(pi.hThread);
#if defined(_DEBUG) && !AWAIT
    std::cout << "DLL loaded, exiting...\n";
#endif
    return 0;
}
