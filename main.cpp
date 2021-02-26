//
// clang++ -std=c++11 CrashHandler.cpp main.cpp
//
#include "CrashHandler.h"

static auto crash() -> void
{
    int * p = ( int * )0;
    *p = 1;
}

static auto bar() -> void
{
    crash();
}

static auto foo() -> void
{
    bar();
}

auto main() -> int
{
    boom::CrashHandler crashHandler{};
    foo();
    return 0;
}
