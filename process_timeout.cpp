#include <windows.h>
#include <iostream>
#include <string>

int main() {
    STARTUPINFO si{};
    si.cb = sizeof(STARTUPINFO);
    PROCESS_INFORMATION pi{};

    std::wstring app = L"C:\\Windows\\System32\\notepad.exe";
    std::wstring cmd = L"\"C:\\Windows\\System32\\notepad.exe\"";
    std::wstring mutableCmd = cmd;

    if (!CreateProcessW(app.c_str(), mutableCmd.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        std::cerr << "CreateProcess failed, error=" << GetLastError() << "\n";
        return 1;
    }

    const DWORD totalTimeoutMs = 10'000;
    const DWORD pollIntervalMs = 200;
    DWORD elapsed = 0;

    while (true) {
        DWORD wait = WaitForSingleObject(pi.hProcess, pollIntervalMs);
        if (wait == WAIT_OBJECT_0) {
            break; // Процес завершився
        } else if (wait == WAIT_TIMEOUT) {
            elapsed += pollIntervalMs;
            if (elapsed >= totalTimeoutMs) {
                std::cerr << "Process exceeded time limit. Terminating...\n";
                if (!TerminateProcess(pi.hProcess, /*exit code*/ 1)) {
                    std::cerr << "TerminateProcess failed, error=" << GetLastError() << "\n";
                }
                // Дочекаємося сигналізації після TerminateProcess
                WaitForSingleObject(pi.hProcess, INFINITE);
                break;
            }
        } else {
            std::cerr << "Wait error, code=" << wait << "\n";
            break;
        }
    }

    DWORD exitCode = 0;
    if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
        std::cout << "Final exitCode=" << exitCode << "\n";
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
}
