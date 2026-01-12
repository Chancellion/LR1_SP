#include <windows.h>
#include <iostream>
#include <string>

int main() {
    STARTUPINFO si{};
    si.cb = sizeof(STARTUPINFO);
    PROCESS_INFORMATION pi{};

    std::wstring app = L"C:\\Windows\\System32\\notepad.exe";
    std::wstring cmdLine = L"\"C:\\Windows\\System32\\notepad.exe\"";

    std::wstring mutableCmd = cmdLine;

    BOOL ok = CreateProcessW(
        app.c_str(),                      // lpApplicationName
        mutableCmd.data(),                // lpCommandLine (mutable)
        nullptr,                          // lpProcessAttributes (безпека)
        nullptr,                          // lpThreadAttributes (безпека)
        FALSE,                            // bInheritHandles
        0,                                // dwCreationFlags
        nullptr,                          // lpEnvironment
        nullptr,                          // lpCurrentDirectory
        &si,                              // lpStartupInfo
        &pi                               // lpProcessInformation
    );

    if (!ok) {
        std::cerr << "CreateProcess failed, error=" << GetLastError() << "\n";
        return 1;
    }

    std::cout << "Process started (PID=" << pi.dwProcessId << "). Waiting...\n";

    DWORD waitResult = WaitForSingleObject(pi.hProcess, INFINITE);
    if (waitResult != WAIT_OBJECT_0) {
        std::cerr << "WaitForSingleObject failed, code=" << waitResult << "\n";
    }

    DWORD exitCode = 0;
    if (!GetExitCodeProcess(pi.hProcess, &exitCode)) {
        std::cerr << "GetExitCodeProcess failed, error=" << GetLastError() << "\n";
    } else {
        // Вивід статусу
        if (exitCode == 0) {
            std::cout << "Process finished successfully (exitCode=0).\n";
        } else {
            std::cout << "Process finished with error (exitCode=" << exitCode << ").\n";
        }
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
}
