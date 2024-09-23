#pragma once

#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <format>
#include <string>

using namespace std::literals; //NOLINT(clang-diagnostic-header-hygiene)

#define ND [[nodiscard]]
#define UU [[maybe_unused]]

#define DELETE_COPY_ROUTINES(CLASS)                                                 \
    CLASS(CLASS const &)            = delete;/*NOLINT(bugprone-macro-parentheses)*/ \
    CLASS &operator=(CLASS const &) = delete /*NOLINT(bugprone-macro-parentheses)*/

#define DELETE_MOVE_ROUTINES(CLASS)                                                      \
    CLASS(CLASS &&) noexcept            = delete; /*NOLINT(bugprone-macro-parentheses)*/ \
    CLASS &operator=(CLASS &&) noexcept = delete  /*NOLINT(bugprone-macro-parentheses)*/

#define DELETE_COPY_MOVE_ROUTINES(CLASS) \
    DELETE_COPY_ROUTINES(CLASS);         \
    DELETE_MOVE_ROUTINES(CLASS)

#define DEFAULT_COPY_ROUTINES(CLASS)                                                  \
    CLASS(CLASS const &)            = default; /*NOLINT(bugprone-macro-parentheses)*/ \
    CLASS &operator=(CLASS const &) = default  /*NOLINT(bugprone-macro-parentheses)*/

#define DEFAULT_MOVE_ROUTINES(CLASS)                                                      \
    CLASS(CLASS &&) noexcept            = default; /*NOLINT(bugprone-macro-parentheses)*/ \
    CLASS &operator=(CLASS &&) noexcept = default  /*NOLINT(bugprone-macro-parentheses)*/

#define DEFAULT_COPY_MOVE_ROUTINES(CLASS) \
    DEFAULT_COPY_ROUTINES(CLASS);         \
    DEFAULT_MOVE_ROUTINES(CLASS)


namespace util {

ND std::wstring GetErrorMessage(DWORD errVal);
ND std::wstring NarrowToWide(char const *ansiString, int len = -1, int codePage = CP_UTF8);
void OpenConsoleWindow();
void DumpToConsole(std::wstring const &msg);
void DumpToConsole(std::wstring_view const &msg);
[[noreturn]] void DumpErrorAndCrash(DWORD errVal);

template <typename ...Types>
void DumpFmt(std::wstring_view const &fmt, Types ...args)
{
    std::wstring outStr = std::vformat(fmt, std::make_wformat_args(args...));
    ::WriteConsoleW(::GetStdHandle(STD_OUTPUT_HANDLE), outStr.c_str(), 
                    static_cast<DWORD>(outStr.size()), nullptr, nullptr);
}

} // namespace util

