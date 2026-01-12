// Parent side (create named event and pass it via command line)
HANDLE readyEvent = CreateEventW(nullptr, TRUE, FALSE, L"Global\\ChildReadyEvent123");
if (!readyEvent) { std::cerr << "CreateEvent failed, error=" << GetLastError() << "\n"; }
// Передаємо ім’я події в аргументах дочці: child.exe --event Global\ChildReadyEvent123
