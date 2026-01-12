#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

struct Child {
    std::wstring app;
    std::wstring cmd;
    PROCESS_INFORMATION pi{};
};

int main() {
    STARTUPINFO si{};
    si.cb = sizeof(STARTUPINFO);

    std::vector<Child> children = {
        { L"C:\\Windows\\System32\\notepad.exe", L"\"C:\\Windows\\System32\\notepad.exe\"" },
        { L"C:\\Windows\\System32\\calc.exe",    L"\"C:\\Windows\\System32\\calc.exe\"" }
        // Додай більше процесів за потреби
    };

    std::vector<HANDLE> handles;

    // Створення процесів
    for (auto& ch : children) {
        std::wstring cmdMutable = ch.cmd;
        BOOL ok = CreateProcessW(
            ch.app.c_str(), cmdMutable.data(),
            nullptr, nullptr, FALSE, 0,
            nullptr, nullptr, &si, &ch.pi
        );
        if (!ok) {
            std::cerr << "CreateProcess failed (" << std::string(ch.app.begin(), ch.app.end())
                      << "), error=" << GetLastError() << "\n";
        } else {
            handles.push_back(ch.pi.hProcess);
            std::cout << "Started PID=" << ch.pi.dwProcessId << "\n";
        }
    }

    // Очікування на всі процеси (по одному чи пакетно)
    const DWORD globalTimeout = 10'000; // 10 секунд
    DWORD startTick = GetTickCount();

    std::vector<bool> finished(children.size(), false);
    size_t remaining = handles.size();

    while (remaining > 0) {
        DWORD elapsed = GetTickCount() - startTick;
        DWORD remainingTimeout = (elapsed >= globalTimeout) ? 0 : (globalTimeout - elapsed);

        DWORD waitRes = WaitForMultipleObjects(
            static_cast<DWORD>(handles.size()), handles.data(),
            FALSE, // батьківський потік розблоковується коли будь-який завершився
            200    // періодичне опитування
        );

        if (waitRes >= WAIT_OBJECT_0 && waitRes < WAIT_OBJECT_0 + handles.size()) {
            DWORD index = waitRes - WAIT_OBJECT_0;
            // Позначаємо завершення
            finished[index] = true;
            remaining--;

            DWORD exitCode = 0;
            GetExitCodeProcess(handles[index], &exitCode);
            std::cout << "Process[" << index << "] exitCode=" << exitCode << "\n";
        } else if (waitRes == WAIT_TIMEOUT) {
            // Перевірка глобального тайм-ауту
            if (GetTickCount() - startTick >= globalTimeout) {
                std::cerr << "Global timeout reached. Terminating remaining processes...\n";
                for (size_t i = 0; i < children.size(); ++i) {
                    if (!finished[i]) {
                        TerminateProcess(children[i].pi.hProcess, 1);
                        WaitForSingleObject(children[i].pi.hProcess, INFINITE);
                        DWORD exitCode = 0;
                        GetExitCodeProcess(children[i].pi.hProcess, &exitCode);
                        std::cout << "Terminated Process[" << i << "] exitCode=" << exitCode << "\n";
                        finished[i] = true;
                        remaining--;
                    }
                }
            }
        } else {
            std::cerr << "WaitForMultipleObjects unexpected result=" << waitRes << "\n";
            break;
        }
    }

    // Закриття дескрипторів
    for (auto& ch : children) {
        if (ch.pi.hThread) CloseHandle(ch.pi.hThread);
        if (ch.pi.hProcess) CloseHandle(ch.pi.hProcess);
    }
    return 0;
}
