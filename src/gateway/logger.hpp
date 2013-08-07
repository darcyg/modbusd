// logger.hpp //

#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_


#include "string.hpp"


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


/**
    @brief Print messages to the log.

    When object of @c log_t class is constructed, it accepts @c name parameter. Instance remembers
    its name, it will be used as prefix in all the messages printed through the instance.
*/
class logger_t {

    public :

        /**
            @brief Formats source location.

            Source location is built from file name and source line: if @c file is not @c NULL,
            basename of the file joined with colon and line number, enclosed into brackets and
            appended with trailing space.

            If @c file is @c NULL, result is empty string.

            Function is used to format log and exception messages.
        */
        static
        string_t
        location(
            char const * file,
            int line
        );

    public :

        /**
            @brief Levels of log messages.

            Borrowed from syslog. See syslog(3). Not all levels are used.
        */
        enum level_t {
            emerg,
            alert,
            crit,
            error,
            warning,
            notice,
            info,
            debug
        };

        logger_t( string_t const & name, bool verbose = true );
        ~logger_t();
        string_t prefix()   const;
        bool     verbose()  const;

        void     verbose( bool set );

        void log( level_t level, string_t const & message );
        void _log( char const * file, int line, level_t level, string_t const & message );

    private :

        string_t _prefix;
        bool     _verbose;

}; // class logger_t


/**
    @brief Source location.

    If @c NDEBUG macro is not defined, @c _SOURCE_LOCATION_ represent both current source file name
    and line number. If @c NDEBUG is defined, @c _SOURCE_LOCATION_ transforms to @c NULL pointer
    and zero, so error messages will not include source location.
*/
#if NDEBUG
    #define _SOURCE_LOCATION_   NULL, 0
#else
    #define _SOURCE_LOCATION_   __FILE__, __LINE__
#endif

/**
    @brief Implementation of @c DLOG, @c ILOG, @c NLOG, @c WLOG, @c ELOG, and @c CLOG.

    Do not use it directly.
*/
#define _LOG( _level_, _message_ ) {                                                               \
    if ( _logger.verbose() || logger_t::_level_ < logger_t::debug ) {                              \
        osstream_t _log_stream_;                                                                   \
        _log_stream_ << _message_;                                                                 \
        _logger._log( _SOURCE_LOCATION_, logger_t::_level_, _log_stream_.str() );                  \
    };                                                                                             \
}


/**
    @brief Print critical message to the log.

    See @c DLOG for usage example.
*/
#define CLOG( msg ) _LOG( crit,    msg );


/**
    @brief Print error message to the log.

    See @c DLOG for usage example.
*/
#define ELOG( msg ) _LOG( error,   msg );


/**
    @brief Print warning message to the log.

    See @c DLOG for usage example.
*/
#define WLOG( msg ) _LOG( warning, msg );


#define NLOG( msg ) _LOG( notice,  msg );


/**
    @brief Print informational message to the log.

    See @c DLOG for usage example.
*/
#define ILOG( msg ) _LOG( info,    msg );


/**
    @brief Print debug message to the log.

    @param msg Message to print. This macro argument is subjet to print to ostream, so it could
        include multiple data items separated with @c <<. See example.

    Example:
    @code
    DLOG( "Opening file `" << path << "'" );
    @endcode
*/
#if NDEBUG
    #define DLOG( msg ) void( 0 )
#else
    #define DLOG( msg ) _LOG( debug, msg );
#endif


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#endif // _LOGGER_HPP_

// end of file //
