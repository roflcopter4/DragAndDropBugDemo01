#include "Common.h"
#include <cassert>
#include <mutex>

namespace util {

ND std::wstring GetErrorMessage(DWORD errVal)
{
    wchar_t buf[512];
    DWORD res = ::FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errVal, LANG_SYSTEM_DEFAULT, buf, std::size(buf), nullptr
    );
    return (res < 2 
            ? L"Unknown error"s
            : std::wstring(buf, res - 2)) +
        std::format(L" (0x{:08X})", errVal);
}

[[noreturn]] void DumpErrorAndCrash(DWORD errVal)
{
    ::MessageBoxW(nullptr, GetErrorMessage(errVal).c_str(), L"Fatal Error", MB_OK | MB_ICONERROR);
    ::ExitProcess(errVal);
}

static void do_OpenConsoleWindow()
{
    ::FreeConsole();
    if (!::AllocConsole()) {
        DWORD err = ::GetLastError();
        WCHAR buf[128];
        swprintf_s(buf, std::size(buf),
                   L"Failed to allocate a console (error %ls). "
                   L"If this happens your computer is probably on fire.",
                   GetErrorMessage(err).data());
        ::MessageBoxW(nullptr, buf, L"Fatal Error", MB_OK | MB_ICONERROR);
        ::exit(1);
    }

    int   ret;
    FILE *conout;
    ret = _wfreopen_s(&conout, L"CONOUT$", L"w", stdout);
    assert(ret == 0);
    ret = _wfreopen_s(&conout, L"CONOUT$", L"w", stderr);
    assert(ret == 0);
    ret = _wfreopen_s(&conout, L"CONIN$", L"r", stdin);
    assert(ret == 0);

    HANDLE hInput  = ::GetStdHandle(STD_INPUT_HANDLE);
    DWORD  conmode = 0;
    ::GetConsoleMode(hInput, &conmode);
    conmode &= ~ENABLE_QUICK_EDIT_MODE;
    conmode &= ~ENABLE_MOUSE_INPUT;
    ::SetConsoleMode(hInput, conmode);
}

void OpenConsoleWindow()
{
    static std::once_flag flag;
    std::call_once(flag, do_OpenConsoleWindow);
}

ND std::wstring NarrowToWide(char const *ansiString, int len, int codePage)
{
    std::wstring ret;
    int n = ::MultiByteToWideChar(codePage, 0, ansiString, len, nullptr, 0);
    ret.resize(n);
    ::MultiByteToWideChar(codePage, 0, ansiString, len, ret.data(), n);
    return ret;
}

void DumpToConsole(std::wstring const &msg)
{
    ::WriteConsoleW(::GetStdHandle(STD_OUTPUT_HANDLE), msg.c_str(), 
                    static_cast<DWORD>(msg.size()), nullptr, nullptr);
}

void DumpToConsole(std::wstring_view const &msg)
{
    ::WriteConsoleW(::GetStdHandle(STD_OUTPUT_HANDLE), msg.data(), 
                    static_cast<DWORD>(msg.size()), nullptr, nullptr);
}

} // namespace util
