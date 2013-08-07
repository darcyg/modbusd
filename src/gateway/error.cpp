// error.cpp //

/**
    @file
    @brief Error handling and reporting implementation.
*/

#include "error.hpp"

#include "logger.hpp"
#include "system.hpp"

#include <cstring>
#include <sstream>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


string_t
str_error(
    int     code
) {

    #if ( _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600 ) && ! _GNU_SOURCE
        char buffer[ 1000 ];
        int rc = strerror_r( code, buffer, sizeof( buffer ) );
        if ( rc == -1 ) {
            int e = errno;
            osstream_t s;
            switch ( e ) {
                case EINVAL : {     // Bad error code.
                    WLOG( "strerror_t( " << code << ") failed with error EINVAL" );
                    s << "Unknown error " << code;
                } break;
                case ERANGE : {     // Oops, buffer is too sort.
                    WLOG( "strerror_t( " << code << ") failed with error ERANGE: buffer is too short" );
                    s << "Error " << code;
                } break;
                default : {
                    WLOG( "strerror_t( " << code << ") failed with unexpected error " << e );
                    s << "Error " << code;
                } break;
            };
            return s.str();
        }; // if
        return buffer;
    #else
        char buffer[ 1000 ];
        return strerror_r( code, buffer, sizeof( buffer ) );
    #endif
}; // str_error


// -------------------------------------------------------------------------------------------------
// err_t class
// -------------------------------------------------------------------------------------------------


err_t::err_t(
    char const *        file,
    int                 line,
    string_t const &    prefix,
    string_t const &    message
) :
    std::runtime_error( logger_t::location( file, line ) + prefix + message ),
    _file( file ),
    _line( line ),
    _msg( message )
{
}; // ctor


err_t::~err_t(
) throw () {
};


string_t
err_t::msg(
) const {
    return _msg;
}; // msg


// -------------------------------------------------------------------------------------------------
// sys_err_t class
// -------------------------------------------------------------------------------------------------


sys_err_t::sys_err_t(
    char const *        file,
    int                 line,
    string_t const &    prefix,
    string_t const &    message,
    string_t const &    function,
    int                 code
) :
    err_t(
        file, line,
        prefix,
        message + ": `" + function + "' failed: Error " + str( code ) + ": " + str_error( code )
    ),
    _code( code )
{
}; // ctor


std::ostream &
operator <<(
    std::ostream &          stream,
    std::exception const &  ex
) {
    stream << ex.what();
    return stream;
};


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


// end of file //
