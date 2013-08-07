// daemon.cpp //

/**
    @file
    @brief Daemon class implementation.
*/

#include "daemon.hpp"

#include "error.hpp"
#include "system.hpp"
#include "thread.hpp"

#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


static char const * const ok = "(ok)";

daemon_t::daemon_t(
    args_t const &    args
) :
    object_t( "daemon" ),
    _args( args ),
    _pid( * this )
{
};


daemon_t::~daemon_t(
) {
    if ( _pid.fd() != -1 ) {
        // We have created pid file. Let us delete it.
        int error = -1;
        WSYSCALL(
            "deleting `" << _args.pid_file << "'",
            "unlink",
            error = unlink( _args.pid_file.c_str() ),
            ! error,
            "deleted"
        );
    }; // if
};


void
daemon_t::run(
) {
    if ( _args.daemonize ) {
        DLOG( "daemonizing..." );
        _parent();
    } else {
        DLOG( "running in foreground..." );
        _init();
        _body();
    }; // if
}; // run


void
daemon_t::_parent(
) {

    // http://www.freedesktop.org/software/systemd/man/daemon.html

    pipe_t c_pipe( "child"  );
    pipe_t g_pipe( "grandchild" );

    // TODO: Sanitize environment?

    pid_t pid = 0;

    ESYSCALL(
        "forking child",
        "fork",
        pid = fork(),
        pid >= 0,
        "child forked " << ( pid == 0 ? "(i am child)" : "(i am parent)" )
    );

    if ( pid == 0 ) {
        _child( c_pipe, g_pipe );
        return;     // TODO: Or exit?
    };

    c_pipe.writer().close();
    g_pipe.writer().close();

    int error = 0;

    ESYSCALL(
        "anti-zombying child",
        "waitpid",
        error = waitpid( pid, NULL, WNOHANG ),
        error != -1,
        "child anti-zombied"
    );

    // Wait until child finishes initialization.
    string_t c_reply = c_pipe.reader().read();
    string_t g_reply = g_pipe.reader().read();

    // Report errors, if any.
    if ( c_reply != ok || g_reply != ok ) {
        osstream_t errors;
        if ( c_reply != ok ) {
            errors << "Child: ";
            if ( c_reply.empty() ) {
                errors << "Died silently";
            } else {
                errors << split( "\n", c_reply );
            };
        }; // if
        if ( g_reply != ok ) {
            if ( ! errors.str().empty() ) {
                errors << "; ";
            };
            errors << "Grandchild: ";
            if ( g_reply.empty() ) {
                errors << "Died silently";
            } else {
                errors << split( "\n", g_reply );
            };
        }; // if
        ERR(
            1,
            "Daemon initialization failed: " << errors.str()
        );
    }; // if

    c_pipe.reader().close();
    g_pipe.reader().close();

}; // _parent


void
daemon_t::_child(
    pipe_t &    c_pipe,
    pipe_t &    g_pipe
) {

    try {

        int error = -1;

        c_pipe.reader().close();
        g_pipe.reader().close();

        // Reopen standard streams.
        // Associate all the standard streams with `/dev/null'.
        file_t null( * this );
        null.open( "/dev/null", O_RDONLY, 0 );
        error = dup2( null.fd(), STDIN_FILENO );
        SYSERR( error < 0, "dup2", "Reopening stdin" );
        null.close();
        null.open( "/dev/null", O_WRONLY, 0 );
        error = dup2( null.fd(), STDOUT_FILENO );
        SYSERR( error < 0, "dup2", "Reopening stdout" );
        error = dup2( null.fd(), STDERR_FILENO );
        SYSERR( error < 0, "dup2", "Reopening stderr" );
        null.close();

        pid_t sid = 0;

        ESYSCALL(
            "creating session",
            "setsid",
            sid = setsid(),
            sid > 0,
            "session " << sid << " created"
        );

        // Ignore unused signals.
        // We ignore signals in child so in grandchild these signals are fro the very beginning.
        // If daemon need any of these signals, it will handle them explicitly.
        thread_t::ignore_signal( SIGHUP   );
        thread_t::ignore_signal( SIGINT   );
        thread_t::ignore_signal( SIGQUIT  );
        thread_t::ignore_signal( SIGUSR1  );
        thread_t::ignore_signal( SIGUSR2  );
        thread_t::ignore_signal( SIGTERM  );
        thread_t::ignore_signal( SIGCHLD  );
        thread_t::ignore_signal( SIGTTIN  );
        thread_t::ignore_signal( SIGTTOU  );
        thread_t::ignore_signal( SIGWINCH );

        pid_t pid = 0;

        ESYSCALL(
            "forking grandchild process",
            "fork",
            pid = fork(),
            pid >= 0,
            "grandchild process forked " << ( pid == 0 ? "(i am grandchild)" : "(i am child)" )
        );

        if ( pid == 0 ) {
            _grandchild( c_pipe, g_pipe );
            return;         // TODO: or exit?
        }; // if

        g_pipe.writer().close();

        ESYSCALL(
            "anti-zombying grandchild",
            "waitpid",
            error = waitpid( pid, NULL, WNOHANG ),
            error != -1,
            "grandchild anti-zombied"
        );

        // We cannot just close pipe: on the parent end ir will be not clear if everything is ok
        // or child silently died. So let us write `ok' to the pipe -- parent will know that we
        // are really ok.
        c_pipe.writer().write( ok );

    } catch ( std::exception const & ex ) {

        c_pipe.writer().write( string_t( ex.what() ) + "\n" );

    } catch ( ... ) {

        c_pipe.writer().write( "Unknown exception occurred\n" );

    }; // try

    c_pipe.writer().close();

}; // _child


void
daemon_t::_grandchild(
    pipe_t & c_pipe,
    pipe_t & g_pipe
) {

    try {

        c_pipe.writer().close();

        _init();

        // Initialization finished successfully -- let parent know everything is ok.
        g_pipe.writer().write( ok );
        g_pipe.writer().close();

    } catch ( std::exception const & ex ) {

        g_pipe.writer().write( string_t( ex.what() ) + "\n" );
        throw;

    } catch ( ... ) {

        g_pipe.writer().write( "Unknown exception occurred\n" );
        throw;

    }; // try

    try {
        _body();
    } catch ( std::exception const & ex ) {
        ELOG( "ex caught in daemon `_body': " << ex.what() );
    } catch ( ... ) {
        ELOG( "unknown ex caught in daemon `_body'" );
    };

}; // _grandchild


void
daemon_t::_init(
) {

    umask( 0 );

    int error = -1;

    ESYSCALL(
        "changing directory to \"/\"",
        "chdir",
        error = chdir( "/" ),
        ! error,
        "directory changed"
    );

    init();

    // TODO: Create threads but wait until parent exit.
    _pid.open( _args.pid_file, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP );
    _pid.lock( F_TLOCK, 0 );
    _pid.write( str( getpid() ) + "\n" );

}; // _init


void
daemon_t::_body(
) {
    body();
}; // _body


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


// end of file //
