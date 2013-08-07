// system.cpp //

#include "system.hpp"

#include "string.hpp"
#include "error.hpp"

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <iomanip>

#include <fcntl.h>
#include <libgen.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


std::ostream &
operator <<(
    std::ostream &  stream,
    file_t const &  file
) {
    stream << file.name() << " (" << file._fd << ")";
    return stream;
};

// -------------------------------------------------------------------------------------------------
// file_t class
// -------------------------------------------------------------------------------------------------

file_t::file_t(
    object_t & parent
) :
    object_t( parent, "anon file" ),
    _fd( -1 )
{
    DLOG( "constructed" );
};


file_t::file_t(
    string_t const &    name,
    int                 fd
) :
    object_t( name ),
    _fd( fd )
{
    DLOG( "constructed" );
};


file_t::file_t(
    file_t const & obj
) :
    object_t( "anon file" ),
    _fd( -1 )
{
    DLOG( "constructed" );
    * this = obj;
};


file_t::~file_t(
) {
    _close( true );
    DLOG( "destructed" );
};


void
file_t::operator =(
    file_t const & file
) {
    close();
    _fd = file._fd;
    DLOG( "grabbing `" << file.name() << "'..." );
    rename( file.name() );
    const_cast< file_t & >( file )._fd = -1;
    const_cast< file_t & >( file ).rename( "anon file" );
};


int
file_t::fd(
) const {
    return _fd;
};


void
file_t::open(
    string_t const & path,
    int              flags,
    mode_t           mode
) {
    close();
    rename( "`" + path + "' file" );
    ESYSCALL(
        "opening",
        "open",
        _fd = ::open( path.c_str(), flags, mode ),
        _fd >= 0,
        "opened, handle #" << _fd
    );
    //~ TODO: _name += " (" + str( _fd ) + ")";
}; // open


void
file_t::socket(
) {
    close();
    ESYSCALL(
        "creating socket",
        "socket",
        _fd = ::socket( AF_UNIX, SOCK_STREAM, 0 ),
        _fd >= 0,
        "socket created, new handle #" << _fd
    );
    rename( "socket #" + str( _fd ) );
}; // socket


void
file_t::bind(
    string_t const &    path
) {
    int rc = -1;
    sockaddr_un addr;
    _init_sockaddr_un( path, & addr );
    ESYSCALL(
        "binding socket to path `" << path << "'",
        "bind",
        rc = ::bind( _fd, reinterpret_cast< sockaddr * >( & addr ), sizeof( addr ) ),
        rc != -1,
        "bound"
    );
}; // bind


void
file_t::listen(
) {
    int rc = -1;
    ESYSCALL(
        "listening",
        "listen",
        rc = ::listen( _fd, 1000 ), // TODO: Third parameter?
        rc != -1,
        "listened"
    );
};


file_t
file_t::accept(
) {
    sockaddr_un addr;
    socklen_t len = sizeof( addr );
    memset( & addr, 0, len );
    int fd = -1;
    ESYSCALL(
        "accepting",
        "accept",
        fd = ::accept( _fd, reinterpret_cast< sockaddr * >( & addr ), & len ),
        fd != -1,
        "accepted, new handle #" << fd
    );
    #if 0
        // Nothing interesting in addr for local sockets: just type of address (AF_UNIX).
        if ( len == sizeof( addr ) ) {
            if ( addr.sun_family == AF_UNIX ) {
                DLOG( "accepted connection from address `" << addr.sun_path << "'" );
            } else {
                WLOG( "accept returned address of unexpected type " << addr.sun_family );
            }; // if
        } else {
            WLOG(
                "accept returned sockaddr struct of " << len << " bytes "
                    << "while expected size is " << sizeof( addr )
            );
        };
    #endif
    return file_t( string_t( "socket #" + str( fd ) ), fd );
}; // accept


void
file_t::connect(
    string_t const & path
) {
    int error = -1;
    sockaddr_un addr;
    _init_sockaddr_un( path, & addr );
    ESYSCALL(
        "connecting",
        "connect",
        error = ::connect( _fd, reinterpret_cast< sockaddr * >( & addr ), sizeof( addr ) ),
        error != -1,
        "connected"
    );
}; // connect


void
file_t::write(
    buffer_t const & bulk
) {
    ssize_t written = -1;
    ESYSCALL(
        "writing " << bulk.size() << " bytes: " << bulk,
        "write",
        written = ::write( _fd, bulk.data(), bulk.size() ),
        written != -1 && size_t( written ) == bulk.size(),
        written << " bytes written"
    );
};


void
file_t::write(
    string_t const & bulk
) {
    ssize_t written = -1;
    ESYSCALL(
        "writing " << bulk.size() << " bytes: " << c_literal( bulk ),
        "write",
        written = ::write( _fd, bulk.data(), bulk.size() ),
        written != -1 && size_t( written ) == bulk.size(),
        written << " bytes written"
    );
};


string_t
file_t::read(
    size_t size
) {
    string_t str;
    ssize_t got   = -1;
    while ( str.size() < size && got != 0 ) {
        char buffer[ 10000 ];
        size_t wanted = std::min( size - str.size(), sizeof( buffer ) );
        ESYSCALL(
            "reading " << wanted << " byte(s)",
            "read",
            got = ::read( _fd, reinterpret_cast< void * >( buffer ), wanted ),
            got != -1,
            got << " byte(s) read: " << buffer_t( buffer, got )
        );
        str.append( buffer, got );
    };
    return str;
};


void
file_t::lock(
    int     function,
    off_t   size
) {
    ESYSCALL(
        "locking" ,
        "lockf",
        int error = ::lockf( _fd, function, size ),
        ! error,
        "locked"
    );
}; // lock


void
file_t::close(
) {
    _close( false );
};


void
file_t::_close(
    bool dtor
) {
    if ( _fd >= 0 ) {
        int error = -1;
        _SYSCALL(
            ! dtor,
            "closing",
            "close",
            error = ::close( _fd ),
            ! error,
            "closed"
        );
        _fd = -1;
    };
}; // _close


void
file_t::_init_sockaddr_un(
    string_t const &    path,
    void *              addr
) {
    sockaddr_un * _addr = reinterpret_cast< sockaddr_un * >( addr );
    memset( _addr, 0, sizeof( * _addr ) );
    _addr->sun_family = AF_UNIX;
    ERR(
        path.size() > sizeof( _addr->sun_path ) - 1,
        "Socket path `" << path << "' is of " << path.size() << " bytes, "
            << "which exceeds allowed max length " << sizeof( _addr->sun_path ) - 1
    );
    path.copy( _addr->sun_path, sizeof( _addr->sun_path ) - 1 );
}; // _init_sockaddr_un


// -------------------------------------------------------------------------------------------------
// pipe_t class
// -------------------------------------------------------------------------------------------------

pipe_t::pipe_t(
    string_t const & name
) :
    object_t( name + " pipe" ),
    _reader( * this ),
    _writer( * this )
{
    int tube[ 2 ] = { -1, -1 };
    int error = -1;
    ESYSCALL(
        "creating",
        "pipe",
        error = ::pipe( tube ),
        ! error,
        "created, new handles #" << tube[ 0 ] << ", #" << tube[ 1 ]
    );
    _reader = file_t( this->name() + " reader end", tube[ 0 ] );
    _writer = file_t( this->name() + " writer end", tube[ 1 ] );
};


// -------------------------------------------------------------------------------------------------
// class timer_t
// -------------------------------------------------------------------------------------------------

logger_t timer_logger( "timer" );

timer_t::timer_t(
    clockid_t clock
) {
    _clock = clock;
    _start.tv_sec = 0;
    _start.tv_nsec = 0;
    _stop.tv_sec = 0;
    _stop.tv_nsec = 0;
};


void
timer_t::start(
) {
    logger_t & _logger = timer_logger;
    int error = clock_gettime( _clock, & _start );
    SYSERR( error, "clock_gettime", "" );
};


void
timer_t::stop(
) {
    logger_t & _logger = timer_logger;
    int error = clock_gettime( _clock, & _stop );
    SYSERR( error, "clock_gettime", "" );
};


double
timer_t::elapsed(
) const {
    return
        double( _stop.tv_sec - _start.tv_sec )
        + 1.0E-9 * double( _stop.tv_nsec - _start.tv_nsec );
};


std::ostream &
operator <<(
    std::ostream &  stream,
    timer_t const & timer
) {
    stream << std::setprecision( 3 ) << timer.elapsed() << " s";
    return stream;
};


// -------------------------------------------------------------------------------------------------
// string utilities
// -------------------------------------------------------------------------------------------------


string_t
abs_path(
    string_t const & path
) {

    static logger_t _logger( __FUNCTION__, false );

    string_t  head = path;
    strings_t tail;

    for ( ; ; ) {
        try {
            head = real_path( head );
            break;
        } catch ( sys_err_t const & ex ) {
            ELOG( "caught: " << ex.what() );
            if ( ex.code() == ENOENT ) {
                tail.push_back( base_name( head ) );
                head = dir_name( head );
            } else {
                ELOG( "rethrowing: " << ex.what() );
                throw;
            };
        };
    };

    while ( ! tail.empty() ) {
        string_t const & item = tail.back();
        if ( item == "." ) {
            // Ignore.
        } else if ( item == ".." ) {
            ERR(
                head == "/",
                "Getting absolute path of " << c_literal( path ) << " failed: Invalid path"
            );
            head = dir_name( head );
        } else {
            head = head + ( head == "/" ? "" : "/" ) + item;
        };
        tail.pop_back();
    };

    return head;
};


string_t
base_name(
    string_t const & path
) {
    // Important! We want buffers contains trailiz zero byte, so simple
    //     buffer_t buffer( path );
    // will not work.
    buffer_t buffer( path.c_str(), path.size() + 1 );
    return basename( buffer.data() );
};


string_t
dir_name(
    string_t const & path
) {
    // Important! We want buffers contains trailiz zero byte.
    buffer_t buffer( path.c_str(), path.size() + 1 );
    return dirname( buffer.data() );
};


string_t
real_path(
    string_t const & path
) {
    static logger_t _logger( __FUNCTION__, false );
    char * buffer = NULL;
    ESYSCALL(
        "getting real path of `" << path << "'",
        "realpath",
        buffer = realpath( path.c_str(), NULL ),
        buffer != NULL,
        "real path of `" << path << "' is `" << buffer << "'"
    );
    string_t result( buffer );
    free( buffer );
    return result;
}; // real_path


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


// end of file //
