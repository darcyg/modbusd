// string.cpp //

/**
    @file
    @brief Implementation of string manipulation utilities.
*/

#include "string.hpp"

#include <cctype>
#include <cstring>
#include <iomanip>
#include <sstream>

#include <libgen.h>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


// -------------------------------------------------------------------------------------------------
// buffer_t class
// -------------------------------------------------------------------------------------------------


buffer_t::buffer_t(
    size_t          size,
    char            fill
) :
    parent_t( size, fill )
{
};


buffer_t::buffer_t(
    char const *    str
) :
    parent_t( str, str + strlen( str ) )
{
};


buffer_t::buffer_t(
    char const *    str,
    size_t          size
) :
    parent_t( str, str + size )
{
};


buffer_t::buffer_t(
    string_t const & str
) :
    parent_t( str.data(), str.data() + str.size() )
{
};


/**
    @brief Print buffer in hex format.

    Content of buffer is printed in braces, 16-byte portions are separated with bars, bytes are
    printed in hex without 0x prefix.
*/
std::ostream &
operator <<(
    std::ostream &      stream,
    buffer_t const &    buffer
) {
    static char const * hex = "0123456789ABCDEF";
    stream << "{";
    for ( size_t i = 0; i < buffer.size(); ++ i ) {
        char ch = buffer[ i ];
        if ( i % 16 == 0 && i != 0 ) {
            stream << " |";
        };
        stream
            << " "
            << hex[ unsigned( ch >> 4 ) & 0x0F ]
            << hex[ unsigned( ch >> 0 ) & 0x0F ];
    }; // for i
    if ( ! buffer.empty() ) {
        stream << " ";
    };
    stream << "}";
    return stream;
}; // <<


/**
    @brief Returns a string looks like C string literal

    Useful to visualize non-printable characters.
*/
string_t
c_literal(
    string_t const & str
) {

    osstream_t res;
    res << "\"";
    for ( size_t i = 0; i < str.size(); ++ i ) {
        char ch = str[ i ];
        // This is ugly hack!
        if ( isprint( ch ) || ch & 0x80 ) {
            res << ch;
        } else {
            switch ( ch ) {
                case '"' : {
                    res << "\\\"";
                } break;
                case '\\' : {
                    res << "\\\\";
                } break;
                #if 0
                    case '\a' : {
                        res << "\\a";
                    } break;
                    case '\b' : {
                        res << "\\b";
                    } break;
                    case '\f' : {
                        res << "\\f";
                    } break;
                    case '\n' : {
                        res << "\\n";
                    } break;
                    case '\r' : {
                        res << "\\r";
                    } break;
                    case '\t' : {
                        res << "\\t";
                    } break;
                    case '\v' : {
                        res << "\\v";
                    } break;
                #endif
                default : {
                    res << "\\x";
                    if ( true ) {
                        // Hex digits are in upper case.
                        static char const * hex = "0123456789ABCDEF";
                        res
                            << hex[ unsigned( ch >> 4 ) & 0x0F ]
                            << hex[ unsigned( ch >> 0 ) & 0x0F ];
                    } else {
                        // C++ style, hex digits in lower case.
                        res
                            << std::setw( 2 ) << std::setfill( '0' ) << std::setbase( 16 )
                                << ( unsigned( ch ) & 0xFF );
                    }; // if
                } break;
            }; // switch
        }; // if
    }; // for i
    res << "\"";

    return res.str();

}; // c_literal


/**
    @brief Perl-like split.

    Splits a string into pieces. Leading empty pieces are preserved, trailing empty pieces are
    deleted.
*/
strings_t
split(
    string_t const & splitter,
    string_t const & string     ///< String to split.
) {
    strings_t result;
    if ( ! string.empty() ) {
        size_t b = 0;
        do {
            size_t e = splitter.empty() ? b + 1 : string.find( splitter, b );
            if ( e == string_t::npos ) {
                e = string.size();
            };
            result.push_back( string.substr( b, e - b ) );
            b = e + splitter.size();
        } while ( b < string.size() );
        while ( ! result.empty() && result.back().empty() ) {
            result.pop_back();
        };
    };
    return result;
};


/**
    @brief Perl-like join.

    Joins separate strings into a signle sring.
*/
string_t
join(
    string_t const & joiner,
    strings_t const & strings   ///< Strings to join.
) {
    string_t result;
    for ( size_t i = 0; i < strings.size(); ++ i ) {
        if ( i > 0 ) {
            result += joiner;
        };
        result += strings[ i ];
    };
    return result;
};


string_t
repeat(
    string_t const & str,
    size_t           n
) {
    string_t result;
    result.reserve( str.size() * n );
    for ( size_t i = 0; i < n; ++ i ) {
        result += str;
    };
    return result;
};


/**
    @brief Returns an uppercased version of string.

    May not respect locale.
*/
string_t
uc(
    string_t const & str
) {
    string_t res;
    res.resize( str.size() );
    for ( size_t i = 0; i < str.size(); ++ i ) {
        res[ i ] = toupper( str[ i ] );
    }; // for i
    return res;
}; // uc


/**
    @brief Returns an lowercased version of string.

    May not respect locale.
*/
string_t
lc(
    string_t const & str
) {
    string_t res;
    res.resize( str.size() );
    for ( size_t i = 0; i < str.size(); ++ i ) {
        res[ i ] = tolower( str[ i ] );
    }; // for i
    return res;
}; // lc


std::ostream &
operator <<(
    std::ostream &      stream,
    strings_t const &   items
) {
    stream << "{";
    if ( ! items.empty() ) {
        for ( size_t i = 0; i < items.size(); ++ i ) {
            if ( i > 0 ) {
                stream << ",";
            };
            stream << " " << c_literal( items[ i ] );
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


// end of file //
