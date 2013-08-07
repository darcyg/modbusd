// thread.hpp //

/**
    @file
    @brief Thread interface.
*/

#ifndef _THREAD_HPP_
#define _THREAD_HPP_


#include "string.hpp"
#include "logger.hpp"
#include "mutex.hpp"
#include "object.hpp"

#include <pthread.h>
#include <signal.h>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


class pool_t;



class thread_t :public object_t {

    friend class master_t;
    friend class worker_t;


    public :        // Signal utilities.

        /**
            @brief Construct sigset from arguments.

            The last argument must be zero!
        */
        static sigset_t sigset( int signal, ... );

    public :        // Process-wide signal stuff.

        /**
            @brief Ignore specified signal.

            This is process-wide setting.
        */
        static void ignore_signal( int signal );

        /**
            @brief Handle specified signal.

            Signal handler does nothing but handled signal (in contrast to ignoed signal) may
            interrupt system call (if signal is not blocked).

            This is process-wide setting.
        */
        static void handle_signal( int signal );

    protected :     // Process-wide signal guts.

        /**
            @brief Process-wide error handler.

            Sets variables @ _int and @c _term.
        */
        static void _signal_handler( int signal );

        /**
            @brief SIGINT was received.

            This variable initialized to zero and set to non-zero if process receives SIGINT.
        */
        static sig_atomic_t _int;

        /**
            @brief SIGTERM was received.

            This variable initialized to zero and set to non-zero if process receives SIGTERM.
        */
        static sig_atomic_t _term;

    protected :     // Thread-specific signal stuff.

        void block_signals( int mode, sigset_t const & set );

        //~ friend class _signal_blocker_t;
        class _signal_blocker_t : public object_t {
            public :
                _signal_blocker_t( int mode, sigset_t const & set );
                ~_signal_blocker_t();
            private :
                _signal_blocker_t( _signal_blocker_t const & );     // Not copyable.
            private :
                bool        _set;
                sigset_t    _old;
        }; // class _signal_blocker_t

    public :

        enum state_t {
            ts_inited,
            ts_started,
            ts_finished,
            ts_joined,
        }; // enum state_t

    public :

        // TODO: Create a thread in ctor, pause it, and let it continue by run.
        thread_t( string_t const & name, sigset_t const & signals, int delay = 0 );
        virtual ~thread_t();

        void run();

        /**
            @brief Wait the thread exit.

            This method should be called eventually, otherwise system resources will leak.
        */
        void join();

        state_t state();

    protected :

        /**
            @brief Thread body.

            Descendants should define this virtual method. This method will be executed in
            a new thread.
        */
        virtual void body() = 0;
        virtual void at_exit();

        void _kill( int signal );

    private :

        bool _is_pending( int signal, ... );

        /**
            @brief Pthread'd thread body.

            `pthread_create' does not accept method, we have to pass a normal function to
            `pthread_create'. We will pass this one.
        */
        static void * _body( void * data );

    private :

        thread_t( thread_t const & );   // Not copyable.

    protected :

        sigset_t        _signals;

    private :

        int             _delay;
        state_t         _state;
        pthread_t       _th;        ///< Pthread's thread handle.

}; // class thread_t


extern __thread thread_t * volatile _thread;


/**
    @brief A worker thread.

    Worker thread is a specialized version of thread: It knows its mater thread, it unblocks only
    one signal, SIGUSR1 (it is expected the worker will exit soon), and it sends the same signal
    back to the master to let it know a worker exited.
*/
class worker_t : public thread_t {

    public :

        worker_t( thread_t & master, string_t const & name, int delay = 0 );

        /**
            @brief Cancel worker.

            This methods should be called by the mater when master wants the worker exit.
        */
        void cancel();

        class cancel_enabler_t : public _signal_blocker_t {
            public :
                explicit cancel_enabler_t();
        }; // class cancel_enabler_t

    protected :

        /**
            @brief Returns true if mater requested the worker to exit.
        */
        bool is_cancelled();

        virtual void at_exit();

    protected :

        thread_t & _master;     ///< Reference to the master thread.

    private :

        static int const cancel_signal = SIGUSR1;
        bool _cancelled;

}; // class worker_t


typedef std::vector< worker_t * > workers_t;


class master_t : public thread_t {
    public :
        master_t( string_t const & name = "master" );
}; // class master_t


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#endif // _THREAD_HPP_


// end of file //
