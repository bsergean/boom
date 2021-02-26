#include "CrashHandler.h"

#include <cxxabi.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <stdlib.h>

#include <unwind.h>

using namespace boom;

CrashHandler::CrashHandler()
{
    registerCrashHandler();
}

void CrashHandler::registerCrashHandler()
{
    const std::vector<int> posix_signals = {
        // Signals for which the default action is "Core".
        SIGABRT, // Abort signal from abort(3)
        SIGBUS,  // Bus error (bad memory access)
        SIGFPE,  // Floating point exception
        SIGILL,  // Illegal Instruction
        SIGIOT,  // IOT trap. A synonym for SIGABRT
        SIGQUIT, // Quit from keyboard
        SIGSEGV, // Invalid memory reference
        SIGSYS,  // Bad argument to routine (SVr4)
        SIGTRAP, // Trace/breakpoint trap
        SIGXCPU, // CPU time limit exceeded (4.2BSD)
        SIGXFSZ, // File size limit exceeded (4.2BSD)
#if defined( BACKWARD_SYSTEM_DARWIN )
        SIGEMT, // emulation instruction executed
#endif
    };
    bool success = true;

    stack_t ss;
    ss.ss_sp = malloc( SIGSTKSZ );
    if ( ss.ss_sp == NULL )
    {
        /* Handle error */;
    }

    ss.ss_size = SIGSTKSZ;
    ss.ss_flags = 0;
    if ( sigaltstack( &ss, NULL ) == -1 )
    {
        /* Handle error */;
    }

    for ( size_t i = 0; i < posix_signals.size(); ++i )
    {
        struct sigaction action;
        memset( &action, 0, sizeof action );
        action.sa_flags =
            static_cast<int>( SA_SIGINFO | SA_ONSTACK | SA_NODEFER | SA_RESETHAND );
        sigfillset( &action.sa_mask );
        sigdelset( &action.sa_mask, posix_signals[i] );
#if defined( __clang__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif
        action.sa_sigaction = &handleSignal;
#if defined( __clang__ )
#pragma clang diagnostic pop
#endif

        int r = sigaction( posix_signals[i], &action, nullptr );
        if ( r < 0 )
            success = false;
    }

    _loaded = success;
}

void CrashHandler::stackWalk( FILE * stream )
{
    _Unwind_Backtrace(
        []( _Unwind_Context * ctx, void * arg ) -> _Unwind_Reason_Code {
            FILE * stream = static_cast<FILE *>( arg );

            uintptr_t pc = _Unwind_GetIP( ctx );

            char buffer[32];
            sprintf( buffer, "0x%lx ", pc );

            fwrite( buffer, 1, strlen( buffer ), stream );
            return _URC_NO_REASON;
        },
        ( void * )stream );

    fprintf( stream, "\n" );
}

void CrashHandler::handleSignal( int, siginfo_t *, void * )
{
    // First write to stderr
    stackWalk( stderr );

    // Now write to a file
    FILE * stream = fopen( "/tmp/addr.log", "w" );
    setvbuf( stream, NULL, _IONBF, 0 );
    stackWalk( stream );
    fclose( stream );

    _exit( EXIT_FAILURE );
}
