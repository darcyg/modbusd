// string.hpp //

/**
    @file
    @brief Few string manipulation utilities.
*/

#ifndef _STRING_HPP_
#define _STRING_HPP_


#include <string>
#include <vector>
#include <sstream>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


/**
    @brief Short synonym. ;-)

    Yes, it is a bit shorter than original type name.
*/
typedef std::string             string_t;


/**
    @brief Short synonym for vector of strings.
*/
typedef std::vector< string_t > strings_t;


/**
    @brief Short synonym.
*/
typedef std::istringstream      isstream_t;


/**
    @brief Short synonym.
*/
typedef std::ostringstream      osstream_t;


/**
    @brief Buffer.

    Strings could be used as buffers, but there are two reasons for special class: (1) String does
    not allow direct access to its storage. (2) I want strings and buffer be printed in different
    manner.
*/
class buffer_t : public std::vector< char > {
    public :
        typedef std::vector< char > parent_t;
        buffer_t( size_t size = 0, char fill = 0 );
        explicit buffer_t( char const * str );
        explicit buffer_t( char const * str, size_t size );
        explicit buffer_t( string_t const & str );
}; // buffer_t


std::ostream &
operator <<(
    std::ostream &      stream,
    buffer_t const &    buffer
);


string_t c_literal( string_t const & str );
strings_t split( string_t const & splitter, string_t const & string );
string_t join( string_t const & joiner, strings_t const & strings );
string_t repeat( string_t const & str, size_t n );
string_t uc( string_t const & str );
string_t lc( string_t const & str );


std::ostream &
operator <<(
    std::ostream &      stream,
    strings_t const &   items
);


template< typename type_t >
string_t
str(
    type_t const &   val
) {
    osstream_t s;
    s << val;
    return s.str();
};


template< typename type_t >
std::ostream &
operator <<(
    std::ostream &                  stream,
    std::vector< type_t > const &   items
) {
    stream << "{";
    if ( ! items.empty() ) {
        for ( size_t i = 0; i < items.size(); ++ i ) {
            if ( i > 0 ) {
                stream << ",";
            };
            stream << " " << items[ i ];
        };
        stream << " ";
    };
    stream << "}";
    return stream;
};


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#endif // _STRING_HPP_


// end of file //
