#pragma once

#include <signal.h>
#include <stdio.h>

//
// Lots of code borrowed from the backward library
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=24724
// https://refspecs.linuxfoundation.org/LSB_1.3.0/gLSB/gLSB/ehframehdr.html
// https://github.com/waweber/uclibc-clang/commit/71c10c484e7dc113396cccb7e503befb759c6346
// https://www.geeksforgeeks.org/addr2line-command-in-linux-with-examples/
// Calling fork from https://access.redhat.com/articles/2921161
// glibc https://code.woboq.org/userspace/glibc/debug/backtrace.c.html
// We need -ffunction-sections -funwind-tables
//

namespace boom
{
class CrashHandler
{
  public:
    CrashHandler();

    void registerCrashHandler();
    bool loaded() const
    {
        return _loaded;
    }

  private:
    bool _loaded;

    static void stackWalk( FILE * stream );
    static void handleSignal( int, siginfo_t * info, void * _ctx );
};
}
