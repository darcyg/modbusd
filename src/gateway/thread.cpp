// thread.cpp //

/**
    @file
    @brief Thread implementation.
*/

#include "thread.hpp"
#include "error.hpp"
#include "logger.hpp"

#include <cassert>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

#include <pthread.h>
#include <signal.h>
#include <unistd.h>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


// -------------------------------------------------------------------------------------------------
// signal utilities
// -------------------------------------------------------------------------------------------------


sigset_t
thread_t::sigset(
    int signal,
    ...
) {
    static logger_t _logger( __FUNCTION__ );
    int error = 0;
    sigset_t set;
    va_list args;
    error = sigemptyset( & set );
    SYSERR( error, "sigemptyset", "Creating signal set" );
    va_start( args, signal );
    while ( signal != 0 ) {
        error = sigaddset( & set, signal );
        SYSERR( error, "sigaddset", "Creating signal set" );
        signal = va_arg( args, int );
    }; // while
    va_end( args );     // Is there a leak?
    return set;
}; // sigset


// -------------------------------------------------------------------------------------------------
// Process-wide signal stuff
// -------------------------------------------------------------------------------------------------


void
thread_t::ignore_signal(
    int signal
) {
    static logger_t _logger( __FUNCTION__ );
    int error = -1;
    struct sigaction sa;
    memset( & sa, 0, sizeof( sa ) );
    sa.sa_handler = SIG_IGN;
    ESYSCALL(
        "ignoring signal #" << signal,
        "sigaction",
        error = sigaction( signal, & sa, NULL ),
        ! error,
        "signal #" << signal << " ignored"
    );
}; // ignore_signal


void
thread_t::handle_signal(
    int signal
) {
    static logger_t _logger( __FUNCTION__ );
    int error = -1;
    struct sigaction sa;
    memset( & sa, 0, sizeof( sa ) );
    sa.sa_handler = _signal_handler;
    ESYSCALL(
        "setting up signal #" << signal << " handler",
        "sigaction",
        error = sigaction( signal, & sa, NULL ),
        ! error,
        "signal #" << signal << " handler set up"
    );
}; // handle_signal


void
thread_t::_signal_handler(
    int signal
) {
    if ( signal == SIGINT ) {
        _int = 1;
    } else if ( signal == SIGTERM ) {
        _term = 1;
    };
}; // _signal_handler


sig_atomic_t thread_t::_int  = 0;
sig_atomic_t thread_t::_term = 0;


// -------------------------------------------------------------------------------------------------
// Thread-specific signal stuff
// -------------------------------------------------------------------------------------------------


void
thread_t::block_signals(
    int                 mode,
    sigset_t const &    set
) {
    EPTHCALL(
        "setting signal mask",
        "pthread_sigmask",
        pthread_sigmask( mode, & set, NULL ),
        "signal mask set"
    );
}; // block_signals


// -------------------------------------------------------------------------------------------------
// thread_t::_signal_blocker_t class
// -------------------------------------------------------------------------------------------------


thread_t::_signal_blocker_t::_signal_blocker_t(
    int                 mode,
    sigset_t const &    set
) :
    object_t( "signal blocker" ),
    _set( false )
{
    _logger.verbose( false );
    EPTHCALL(
        "setting signal mask",
        "pthread_sigmask",
        pthread_sigmask( mode, & set, & _old ),
        "signal mask set"
    );
    _set = true;
}; // ctor


thread_t::_signal_blocker_t::~_signal_blocker_t(
) {
    if ( _set ) {
        WPTHCALL(
            "restoring signal mask",
            "pthread_sigmask",
            pthread_sigmask( SIG_SETMASK, & _old, NULL ),
            "signal mask restored"
        );
    }; // if
}; // dtor


// -------------------------------------------------------------------------------------------------
// thread_t class
// -------------------------------------------------------------------------------------------------


thread_t::thread_t(
    string_t const & name,
    sigset_t const & signals,
    int              delay
) :
    object_t( name ),
    _signals( signals ),
    _delay( delay ),
    _state( ts_inited ),
    _th( 0 )
{
    assert( delay >= 0 );
    ILOG( "Delay = " << _delay << " s" );
};


thread_t::~thread_t(
) {
};


void
thread_t::run(
) {
    ERR( _state != ts_inited, "Internal error: Attempt to restart thread" );
    EPTHCALL(
        "starting",
        "pthread_create",
        pthread_create( & _th, NULL, _body, this ),
        "started"
    );
    _state = ts_started;
}; // run


void
thread_t::join(
) {
    if ( _state == ts_started || _state == ts_finished ) {
        void * retval = NULL;
        EPTHCALL(
            "joining",
            "pthread_join",
            pthread_join(  _th, & retval ),
            "joined, retval is " << retval
        );
        _state = ts_joined;
    };
}; // join


thread_t::state_t
thread_t::state(
) {
    if ( _state == ts_started ) {
        _kill( 0 );
    }; // if
    return _state;
}; // state


void
thread_t::at_exit(
) {
};


void
thread_t::_kill(
    int signal
) {
    //~ if ( _state == ts_started ) {
        EPTHCALL(
            "someone sending me signal " << signal,
            "pthread_kill",
            pthread_kill( _th, signal );
                if ( _error_ == ESRCH ) { // No such process
                    _state = ts_finished;
                    _error_ = 0;
                },
            "someone sent me signal " << signal
        );
    //~ };
}; // _kill


bool
thread_t::_is_pending(
    int signal,
    ...
) {
    int rc = 0;
    bool is_pending = false;
    sigset_t pending_signals;
    va_list args;
    rc = sigpending( & pending_signals );
    SYSERR( rc < 0, "sigpending", "Getting pending signals" );
    va_start( args, signal );
    while ( ! is_pending && signal != 0 ) {
        rc = sigismember( & pending_signals, signal );
        SYSERR( rc < 0, "sigismember", "Check if signal #" << signal << " pending" );
        signal = va_arg( args, int );
    }; // while
    va_end( args );     // TODO: Is there a leak?
    return is_pending;
}; // _is_pending


void *
thread_t::_body(
    void * data
) {

    int status = 0;
    thread_t * thread = reinterpret_cast< thread_t * >( data );
    logger_t & _logger = thread->_logger;
    _thread = thread;

    // Call the thread body.
    try {
        DLOG( "sleeping " << thread->_delay << " s..." );
        {
            _signal_blocker_t unblocker( SIG_UNBLOCK, thread->_signals );
            sleep( thread->_delay );
        }
        thread->body();
    } catch ( sys_err_t const & ex ) {
        if ( ex.code() == EINTR ) {
            ELOG(
                "ex thrown in thread `body' routine caught: " << ex.what() << "; "
                    << "it shouldn't be a problem"
            );
            status = status & 0x01;
        } else {
            ELOG( "ex thrown in thread `body' routine caught: " << ex.what() );
            status = status & 0x02;
        };
    } catch ( std::exception const & ex ) {
        ELOG( "ex thrown in thread `body' routine caught: " << ex.what() );
        status = status & 0x02;
    } catch ( ... ) {
        ELOG( "unknown ex thrown in thread `body' routine caught" );
        status = status & 0x02;
    }; // if

    // Call at_exit handler.
    try {
        thread->at_exit();
    } catch ( std::exception const & ex ) {
        ELOG( "ex thrown in thread `at_exit' caught" << ex.what() );
        status = status & 0x04;
    } catch ( ... ) {
        ELOG( "unknown ex thrown in thread `at_exit' routine caught" );
        status = status & 0x04;
    };

    return reinterpret_cast< void * >( status );

}; // _body


__thread thread_t * volatile _thread = NULL;


// -------------------------------------------------------------------------------------------------
// worker_t class
// -------------------------------------------------------------------------------------------------


worker_t::worker_t(
    thread_t &       master,
    string_t const & name,
    int              delay
) :
    thread_t( name, sigset( cancel_signal, 0 ), delay ),
    _master( master ),
    _cancelled( false )
{
};


void
worker_t::cancel(
) {
    _cancelled = true;
    _kill( cancel_signal );
}; // cancel


bool
worker_t::is_cancelled(
) {
    return _cancelled;
}; // is_signaled


void
worker_t::at_exit(
) {
    _master._kill( cancel_signal );
}; // at_exit


worker_t::cancel_enabler_t::cancel_enabler_t(
) :
    _signal_blocker_t( SIG_UNBLOCK, sigset( cancel_signal, 0 ) )
{
}; // ctor


// -------------------------------------------------------------------------------------------------
// master_t class
// -------------------------------------------------------------------------------------------------


master_t::master_t(
    string_t const & name
) :
    thread_t( name, sigset( SIGINT, SIGTERM, SIGUSR1, 0 ) )
{
    _th = pthread_self();
    _state = ts_started;
};


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


// end of file //
