// error.hpp //

/**
    @file
    @brief Error handling and reporting interface.
*/

#ifndef _ERROR_HPP_
#define _ERROR_HPP_


#include "string.hpp"
#include "logger.hpp"

#include <cerrno>
#include <stdexcept>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


string_t str_error( int code );

std::ostream &
operator <<(
    std::ostream &          stream,
    std::exception const &  ex
);


/**
    @brief Exception base class.

    All the generated exceptions are descendants of this class.

    Do not throw exception directly, use @c ERR macro instead. The macro automates few actions:
    it automatically specifies @c file, @c line, and @c prefix arguments, and writes error message
    to the log.
*/
class err_t : public std::runtime_error {

    public :

        err_t(
            char const *        file,       ///< Source file where exception is thrown.
            int                 line,       ///< Source line where exception is thrown.
            string_t const &    prexix,
            string_t const &    message     ///< Message.
        );

        virtual ~err_t() throw ();

        /**
            @brief Returns short error message.

            @c what returns full error message whic includes source location and prefix. Short error
            message returned by this method is constructor's @ message argument as-is, withot
            source location and prefix.
        */
        virtual string_t msg() const;

    private :

        char const *    _file;
        int             _line;
        string_t        _msg;

}; // class error_t


/**
    @brief Write error message to the log and throw an exception of @c err_t class.

    @param _cond_ Error condition.
    @param _msg_  Error message. This macro argument is seent to output stream, so it may include
        multiply elements separated by @c <<. See example.

    Example:
    @code
    ERR(
        count < 0,
        "Bad count specified: " << count << "; it must be >= 0!"
    );
    @endcode
*/
#define ERR( _cond_, _msg_ ) {                                                                     \
    if ( _cond_ ) {                                                                                \
        osstream_t _stream_;                                                                       \
        _stream_ << _msg_;                                                                         \
        err_t _err_( _SOURCE_LOCATION_, _logger.prefix(), _stream_.str() );                        \
        _logger._log(                                                                              \
            _SOURCE_LOCATION_, logger_t::error, _err_.msg() + " (throwing)"                        \
        );                                                                                         \
        throw _err_;                                                                               \
    };                                                                                             \
}

/**
    @brief System error.

    Constructor takes system error code and converts it to human-readable errro message.
*/
class sys_err_t : public err_t {
    public :
        sys_err_t(
            char const *        file,
            int                 line,
            string_t const &    prefix,
            string_t const &    message,
            string_t const &    function,
            int                 code
        );

        int code() const {
            return _code;
        } ;
    private :

        int     _code;

}; // class sys_err_t


#define SYSERR( _cond_, _func_, _msg_ ) {                                                          \
    if ( _cond_ ) {                                                                                \
        int _error_ = errno;                                                                       \
        osstream_t _stream_;                                                                       \
        _stream_ << _msg_;                                                                         \
        sys_err_t _err_( _SOURCE_LOCATION_, _logger.prefix(), _stream_.str(),  _func_, _error_ );  \
        _logger._log( _SOURCE_LOCATION_, logger_t::error, _err_.msg() + " (throwing)" );           \
        throw _err_;                                                                               \
    };                                                                                             \
}


#define SYSWRN( _cond_, _func_, _msg_ ) {                                                          \
    if ( _cond_ ) {                                                                                \
        int _error_ = errno;                                                                       \
        osstream_t _stream_;                                                                       \
        _stream_ << _msg_;                                                                         \
        sys_err_t _err_( _SOURCE_LOCATION_, _logger.prefix(), _stram_.str(),  _func_, _error_ );   \
        _logger._log( _SOURCE_LOCATION_, logger_t::error, _err_.msg() + " (not throwing)" );       \
    };                                                                                             \
}


/**
    @brief This is implementation of @c ESYSCALL and @c WSYSCALL.
*/
#define _SYSCALL( _ex_, _action_, _func_, _stmt_, _success_, _result_ ) {                          \
    DLOG( _action_ << "..." );                                                                     \
    _stmt_;                                                                                        \
    if ( ! ( _success_ ) ) {                                                                       \
        int _error_ = errno;                                                                       \
        osstream_t _stream_;                                                                       \
        _stream_ << _action_;                                                                      \
        sys_err_t _err_( _SOURCE_LOCATION_, _logger.prefix(), _stream_.str(), _func_, _error_ );   \
        _logger._log(                                                                              \
            _SOURCE_LOCATION_,                                                                     \
            logger_t::error,                                                                       \
            _err_.msg() + ( _ex_ ? " (throwing)" : " (not throwing)" )                             \
        );                                                                                         \
        if ( _ex_ ) {                                                                              \
            throw _err_;                                                                           \
        };                                                                                         \
    };                                                                                             \
    DLOG( _result_ );                                                                              \
}


/**
    @brief Call a system or library function, throw an exception in case of failure.

    This macro intended to call system functions which fails with setting @c errno. The macro
    calls the specified function and checks the result. If function fails, the macro prints
    error message to log and throws an exception. Additionally, before and after call the macro
    logs debug messages.

    Usage:
    @code
    int fd = -1;
    ESYSCALL(
        "opening file `" << path << "'",    // Leading debug log message.
        "open",                             // Function name.
        fd = open( ... ),                   // Statement.
        fd != -1,                           // Success condition.
        "file opened"                       // Trailing debug log message.
    );
    @endcode
*/
#define ESYSCALL( _action_, _func_, _stmt_, _success_, _result_ )                                  \
    _SYSCALL( 1, _action_, _func_, _stmt_, _success_, _result_ )


/**
    @brief The same as @c ESYSCALL but does not throw an exception.

    Error, if any, is recorded to the log anyway.
*/
#define WSYSCALL( _action_, _func_, _stmt_, _success_, _result_ )                                  \
    _SYSCALL( 0, _action_, _func_, _stmt_, _success_, _result_ )


#define PTHERR( _msg_, _func_, _code_ ) {                                                          \
    if ( _code_ ) {                                                                                \
        osstream_t _stream_;                                                                       \
        _stream_ << _msg_;                                                                         \
        sys_err_t _err_( _SOURCE_LOCATION_, _logger.prefix(), _stream_.str(), _func_, _code_ );    \
        _logger._log( _SOURCE_LOCATION_, logger_t::error, _err_.msg() + "(throwing)" );            \
        throw _err_;                                                                               \
    };                                                                                             \
}


#define PTHWRN( _msg_, _func_, _code_ ) {                                                          \
    if ( _code_ ) {                                                                                \
        osstream_t _stream_;                                                                       \
        _stream_ << _msg_;                                                                         \
        sys_err_t _err_( _SOURCE_LOCATION_, _logger.prefix(), _stream_.str(), _func_, _code_ );    \
        _logger._log( _SOURCE_LOCATION_, logger_t::error, _err_.msg() + "(not throwing)" );        \
    };                                                                                             \
}


/**
    @brief Implementation of @c EPTHCALL and @c WPTHCALL.
*/
#define _PTHCALL( _ex_, _action_, _func_, _stmt_, _result_ ) {                                     \
    DLOG( _action_ << "..." );                                                                     \
    int _error_ = _stmt_;                                                                          \
    if ( _error_ ) {                                                                               \
        osstream_t _stream_;                                                                       \
        _stream_ << _action_;                                                                      \
        sys_err_t _err_( _SOURCE_LOCATION_, _logger.prefix(), _stream_.str(), _func_, _error_ );   \
        _logger._log(                                                                              \
            _SOURCE_LOCATION_,                                                                     \
            logger_t::error,                                                                       \
            _err_.msg() + ( _ex_ ? " (throwing)" : " (not throwing)" )                             \
        );                                                                                         \
        if ( _ex_ ) {                                                                              \
            throw _err_;                                                                           \
        };                                                                                         \
    };                                                                                             \
    DLOG( _result_ );                                                                              \
}


/**
    @brief Call pthread function, throw exception in case of failure.

    Like @c ESYSCALL but for pthread calls. Pthread function are not like other system calls.
    Pthread functions always return error code instead of setting errno.

    @code
    EPTHCALL(
        "creating thread " << count,            // Leading debug mesasage.
        "pthread_create",                       // Pthread function name.
        pthread_create( ... ),                  // Pthread function call.
        "thread " << count << " created"        // Trailing debug message.
    );
    @endcode
*/
#define EPTHCALL( _action_, _func_, _stmt_, _result_ )                                             \
    _PTHCALL( 1, _action_, _func_, _stmt_, _result_ )


/**
    @brief Call pthread function, log error, if any, but do not throw exception.

    This macro is similar to @c EPTHCALL, but does not throw exceptions.
*/
#define WPTHCALL( _action_, _func_, _stmt_, _result_ )                                             \
    _PTHCALL( 0, _action_, _func_, _stmt_, _result_ )


#define STATIC_ASSERT( cond ) \
    static void * _STATIC_ASSERT_( __LINE__ )[ (cond) ? 1 : -1 ] = { & _STATIC_ASSERT_( __LINE__ ) };
#define _STATIC_ASSERT_( n ) __STATIC_ASSERT__( n )
#define __STATIC_ASSERT__( n ) _static_assert_ ## n ## _


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#endif // _ERROR_HPP_

// end of file //
