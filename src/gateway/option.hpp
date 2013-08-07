// option.hpp //

#ifndef _OPTION_HPP_
#define _OPTION_HPP_


#include "string.hpp"

#include <climits>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


void show_opts();
string_t opt_name( string_t const & name );
string_t get_opt( string_t const & name, string_t const & dflt = "" );
long get_int_opt( string_t const & name, long dflt = 0, long min = LONG_MIN, long max = LONG_MAX );
bool get_bool_opt( string_t const & name, bool dflt = false );


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#endif // _OPTION_HPP_

// end of file //
