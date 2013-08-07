// logger.cpp //


#include "logger.hpp"

#include "error.hpp"
#include "option.hpp"
#include "system.hpp"
#include "thread.hpp"

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <cstdio>

#include <syslog.h>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {

/**
    @brief Initializes syslog at the first log usage.

    We cannot initialize log in a constructor of static object because log may be used in
    constructors of other static objects (search for "static initialization order fiasco").
    So let us initialize log at the first usage, and take care about multithreaded applications too.
*/
static
void
init(
) {

    // `syslog' should be initialized once.
    // Cannot use `mutex_t' here because: (1) `mutex_t' uses logger,
    // (2) we need statically initialized mutex.
    static pthread_mutex_t mutex  = PTHREAD_MUTEX_INITIALIZER;
    static bool volatile   inited = false;

    if ( ! inited ) {

        int error;

        static string_t const action = "syslog initialization";

        error = pthread_mutex_lock( & mutex );
        // Cannot use `PTHERR' macro here because the macro uses logger.
        if ( error ) {
            throw sys_err_t( _SOURCE_LOCATION_, "", action, "pthread_mutex_lock", error );
        };

        if ( ! inited ) {
            int options = LOG_CONS | LOG_PID;
            // We cannot use `get_opt' (or `get_bool_opt') here, because they use logger.
            if ( getenv( opt_name( "LOG_PERROR" ).c_str() ) ) {
                options |= LOG_PERROR;
            }; // if
            openlog( NULL, options, LOG_USER );
        };

        inited = true;

        error = pthread_mutex_unlock( & mutex );
        if ( error ) {
            throw sys_err_t( _SOURCE_LOCATION_, "", action, "pthread_mutex_unlock", error );
        };

    };

}; // init


// -------------------------------------------------------------------------------------------------
// logger_t class
// -------------------------------------------------------------------------------------------------


string_t
logger_t::location(
    char const * file,
    int line
) {
    osstream_t location;
    if ( file != NULL ) {
        location << "[" << base_name( file ) << ":" << line << "] ";
    };
    return location.str();
};


logger_t::logger_t(
    string_t const &    name,
    bool                verbose
) :
    _prefix( name.empty() ? "" : name + ": " ),
    _verbose( verbose )
{
}; // ctor


logger_t::~logger_t(
) {
    // Closing syslog is optional, we will not do it.
};


string_t
logger_t::prefix(
) const {
    return _prefix;
};


bool
logger_t::verbose(
) const {
    return _verbose;
};


void
logger_t::verbose(
    bool set
) {
    _verbose = set;
};


void
logger_t::log(
    level_t             level,
    string_t const &    message
) {
    _log( NULL, 0, level, message );
};


struct level_t {
    int             id;
    char const *    name;
};


static level_t const levels[] = {
    { LOG_EMERG,   "x" },
    { LOG_ALERT,   "a" },
    { LOG_CRIT,    "c" },
    { LOG_ERR,     "e" },
    { LOG_WARNING, "w" },
    { LOG_NOTICE,  "n" },
    { LOG_INFO,    "i" },
    { LOG_DEBUG,   "#" }
}; // levels


void
logger_t::_log(
    char const *        file,
    int                 line,
    level_t             level,
    string_t const &    message
) {
    init();
    string_t thread = ( _thread == NULL ? "master" : _thread->name() );
    string_t prefix = _prefix;
    if ( prefix.substr( 0, thread.size() + 2 ) == thread + ": " ) {
        prefix.erase( 0, thread.size() + 2 );
    };
    syslog(
        levels[ level ].id,
        "%s %s(%s) %s%s",
        levels[ level ].name,
        location( file, line ).c_str(),
        thread.c_str(),
        prefix.c_str(),
        message.c_str()
    );
}; // _log


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


// end of file //
