// option.cpp //


#include "option.hpp"
#include "logger.hpp"


#include <cassert>
#include <cerrno>   // program_invocation_short_name defined there.
#include <climits>
#include <cstdlib>
#include <cstdio>

#include <unistd.h>     // environ declared there.


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


static
string_t
prefix(
) {
    return uc( program_invocation_short_name ) + "_";
};


string_t
opt_name(
    string_t const & name
) {
    return prefix() + name;
};


void
show_opts(
) {
    logger_t logger( "show_opts" );
    string_t const p = prefix();
    for ( size_t i = 0; environ[ i ] != NULL; ++ i ) {
        string_t var( environ[ i ] );
        if ( var.substr( 0, p.size() ) == p ) {
            logger.log( logger_t::info, var );
        };
    };
};


string_t
get_opt(
    string_t const &    name,
    string_t const &    dflt
) {
    //~ DLOG( "Getting option `" << name << "'..." );
    char const * var = getenv( opt_name( name ).c_str() );
    return ( var == NULL ? dflt : var );
};


long
get_int_opt(
    string_t const &    name,
    long                dflt,
    long                min,
    long                max
) {
    assert( min <= dflt );
    assert( dflt <= max );
    logger_t _logger( "get_int_opt" );
    string_t opt = get_opt( name );
    if ( opt == "" ) {
        return dflt;
    }; // if
    errno = 0;
    char * end = NULL;
    long value = strtol( opt.c_str(), & end, 10 );
    int e = errno;
    if ( end[ 0 ] != 0 ) {
        WLOG(
            "Environment variable `" << opt_name( name ) << "' "
                << "contains `" << opt << "' "
                << "which is not valid integer number; "
                << "default value `" << dflt << "' "
                << "will be used istead"
        );
        return dflt;
    }; // if
    if ( e == ERANGE ) {
        if ( value == LONG_MIN && min == LONG_MIN ) {
            WLOG(
                "Environment variable `" << opt_name( name ) << "' "
                << "contains value `" << opt << "' "
                << "which is too small for `long' datatype; "
                << "`" << value << "' will be used instead"
            );
        } else if ( value == LONG_MAX && max == LONG_MAX ) {
            WLOG(
                "Environment variable `" << opt_name( name ) << "' "
                << "contains value `" << opt << "' "
                << "which is too big for `long' datatype; "
                << "`" << value << "' will be used instead"
            );
        };
    };
    if ( value < min ) {
        value = min;
        WLOG(
            "Environment variable `" << opt_name( name ) << "' "
            << "contains value `" << opt << "' "
            << "which is less than allowed minimum `" << min << "'; "
            << "the minimum value will be used instead"
        );
    } else if ( value > max ) {
        value = max;
        WLOG(
            "Environment variable `" << opt_name( name ) << "' "
            << "contains value `" << opt << "' "
            << "which is great than allowed maximum `" << max << "'; "
            << "the maximum value will be used instead"
        );
    };
    return value;
};



bool
get_bool_opt(
    string_t const &    name,
    bool                dflt
) {
    logger_t _logger( "get_bool_opt" );
    string_t opt = lc( get_opt( name ) );
    if ( opt == "" ) {
        return dflt;
    }; // if
    if ( opt == "1" || opt == "true" || opt == "on" ) {
        return true;
    };
    if ( opt == "0" || opt == "false" || opt == "off" ) {
        return false;
    };
    WLOG(
        "Environment variable `" << opt_name( name ) << "' "
            << "contains `" << opt << "' "
            << "which is not valid boolean value; "
            << "default value `" << ( dflt ? "true" : "false" ) << "' "
            << "will be used istead"
    );
    return dflt;
};



}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


// end of file //
