// system.hpp //

#ifndef _SYSTEM_HPP_
#define _SYSTEM_HPP_


#include "object.hpp"
#include "string.hpp"

#include <climits>

#include <sys/types.h>
#include <time.h>
#include <unistd.h>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {

/**
    @brief File abstraction.

    This is not file on disk, it is rather Unix file descriptor. It could be either file on disk,
    standard input or output stream, or socket.

    Methods are thin wrappers around system calls. If system call fails, wrapper throws an
    exception.

    Destructor closes filehandle.
*/
class file_t : public object_t {
    friend std::ostream & operator <<( std::ostream &, file_t const & );
    public :
        explicit file_t( object_t & parent );
        file_t( string_t const & name, int _fd );
        file_t( file_t const & obj );
        ~file_t();
        int fd() const;
        void operator =( file_t const & file );
        void forget();
        void open( string_t const & path, int flags, mode_t mode );
        void socket();
        void bind( string_t const & path );
        void listen();
        file_t accept();
        void connect( string_t const & path );
        void close();
        string_t read( size_t size = SSIZE_MAX );
        void     write( buffer_t const & bulk );
        void     write( string_t const & bulk );
        void     lock( int function, off_t size );
    private :
        void _close( bool dtor );
        void _init_sockaddr_un( string_t const & path, void * addr );
    private :
        int       _fd;      ///< File descriptor.
}; // class file_t


class pipe_t : object_t {

    public :

        pipe_t( string_t const & name );
        file_t & reader() { return _reader; };
        file_t & writer() { return _writer; };

    private :

        pipe_t( pipe_t const & obj );   // Not copyable.

    private :

        file_t _reader;
        file_t _writer;

}; // class pipe_t


class timer_t {
    public :
        timer_t( clockid_t id = CLOCK_MONOTONIC );
        void start();
        void stop();
        double elapsed() const;
    private :
        clockid_t   _clock;
        timespec    _start;
        timespec    _stop;
}; // class timer_t


std::ostream & operator <<( std::ostream & stream, timer_t const & timer );


/**
    @brief Return absolute path.

    In contrast to real_path, this function works with non-existing files too.
*/
string_t abs_path( string_t const & path );


/**
    @brief Return base (file) name.
*/
string_t base_name( string_t const & path );


/**
    @brief Return directory name.
*/
string_t dir_name( string_t const & path );


/**
    @brief Return real path.

    @b Note: This function does not work with non-existing files.
*/
string_t real_path( string_t const & path );


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#endif // _SYSTEM_HPP_

// end of file //
