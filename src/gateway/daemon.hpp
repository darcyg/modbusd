// daemon.hpp //

/**
    @file
    @brief Daemon interface.
*/

#ifndef _DAEMON_HPP_
#define _DAEMON_HPP_


#include "logger.hpp"
#include "object.hpp"
#include "system.hpp"
#include "thread.hpp"


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


/**
    @brief Daemonization implementation.

    This is a virtual class. It implements low-level daemonization, but does not perform any real
    work. Descendatns should define @c _init and @c _body methods.
*/
class daemon_t : public object_t {

    public :

        /**
            @brief Daemon arguments.
        */
        struct args_t {

            string_t name;

            /**
                @brief Whether to create a daemon.

                If false, daemon will not be created. Instead, process will continue in foreground.
                Useful for debugging.
            */
            bool daemonize;

            /**
                @brief Path to pid file.

                Daemon will write its pid to that file.
            */
            string_t pid_file;

        }; // struct args_t

    public :

        daemon_t( args_t const & args );
        virtual ~daemon_t();

        void run();

    protected :

        /**
            @brief Initialization part.

            Descendants should define this function. It is executed in daemon, but before parent
            process exits. An exception thrown in this part will be sent to parent process and
            reported to the user.
        */
        virtual void init() = 0;

        /**
            @brief Body part.

            Descendants should define this function. It is executed after parent process exit.
            An exceptiont thrown in this part will be written to log, and daemon will exit.
        */
        virtual void body() = 0;

    private :

        void _parent();
        void _child( pipe_t & c_pipe, pipe_t & g_pipe );
        void _grandchild( pipe_t & c_pipe, pipe_t & g_pipe );

        void _init();
        void _body();

    protected :

        args_t const    _args;
        file_t          _pid;

}; // daemon_t


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#endif // _DAEMON_HPP_

// end of file //
