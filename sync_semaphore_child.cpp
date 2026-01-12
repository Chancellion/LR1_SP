// Child side (signal readiness)
HANDLE ev = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"Global\\ChildReadyEvent123");
if (ev) {
    // ... виконати ініціалізацію ...
    SetEvent(ev);
    CloseHandle(ev);
}
    