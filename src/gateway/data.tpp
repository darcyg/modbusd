// data.tpp //

/**
    @file
    @brief Template definitions for data.hpp.
*/

#ifndef _DATA_TPP_
#define _DATA_TPP_


#include "string.hpp"
#include "error.hpp"

#include <istream>
#include <ostream>
#include <vector>
#include <cstring>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


// -------------------------------------------------------------------------------------------------
// fixed-size strings.
// -------------------------------------------------------------------------------------------------


template<
    size_t n
>
fstring_t< n >::fstring_t(
) {
    STATIC_ASSERT( sizeof( * this ) == sizeof( bulk ) );
    STATIC_ASSERT( sizeof( * this ) % alignment == 0 );
    if ( n == sizeof( int ) ) {
        reinterpret_cast< int & >( bulk ) = 0;
    } else {
        memset( & bulk, 0, sizeof( bulk ) );
    }; // if
};


template<
    size_t n
>
fstring_t< n >::fstring_t(
    string_t const & str
) {
    memset( & bulk, 0, sizeof( bulk ) );
    strncpy( bulk, str.c_str(), sizeof( bulk ) );
};


template<
    size_t n
>
char const *
fstring_t< n >::data(
) const {
    return bulk;
};


template<
    size_t n
>
string_t
fstring_t< n >::str(
) const {
    return string_t( bulk, n );
};


template<
    size_t n
>
bool
fstring_t< n >::operator ==(
    fstring_t< n > const & rhs
) const {
    if ( n == sizeof( int ) ) {
        return reinterpret_cast< int const & >( bulk ) == reinterpret_cast< int const & >( rhs.bulk );
    } else {
        for ( size_t i = 0; i < n; ++ i ) {
            if ( bulk[ i ] != rhs.bulk[ i ] ) {
                return false;
            };
        };
    }; // if
    return true;
};


template<
    size_t n
>
bool
fstring_t< n >::operator !=(
    fstring_t< n > const & rhs
) const {
    return ! ( * this == rhs );
};


template<
    size_t n
>
bool
fstring_t< n >::operator >(
    fstring_t< n > const & rhs
) const {
    if ( n == sizeof( int ) ) {
        // NOTE: This comparison depend on host byte order.
        // But it does not matter, we use comparison for dates, not for marks.
        return reinterpret_cast< int const & >( bulk ) > reinterpret_cast< int const & >( rhs.bulk );
    } else {
        for ( size_t i = 0; i < n; ++ i ) {
            if ( bulk[ i ] != rhs.bulk[ i ] ) {
                return bulk[ i ] > rhs.bulk[ i ];
            };
        };
    }; // if
    return false;
};


template<
    size_t n
>
size_t
fstring_t< n >::size(
) const {
    return n;
};


template<
    size_t n
>
void
fstring_t< n >::write(
    std::ostream & stream
) const {
    STATIC_ASSERT( sizeof( bulk ) % alignment == 0 );
    stream.write( bulk, n );
};


template<
    size_t n
>
void
fstring_t< n >::read(
    std::istream & stream
) {
    STATIC_ASSERT( sizeof( bulk ) % alignment == 0 );
    stream.read( bulk, sizeof( bulk ) );
};


template<
    size_t n
>
void
fstring_t< n >::read(
    std::istream & stream
) const {
    logger_t _logger( "TMP" );
    mark_t item;
    item.read( stream );
    ERR(
        item != * this,
        c_literal( this->str() ) << " is expected "
            << " but read " << c_literal( item.str() )
    );
}; // read


template<
    size_t n
>
std::ostream &
operator <<(
    std::ostream &          stream,
    fstring_t< n > const &   item
) {
    stream << c_literal( item.bulk );
    return stream;
};


template<
    typename item_t
>
void
write_packet(
    std::ostream &                  stream,
    std::vector< item_t > const &   items,
    bool                            use_eors = true

) {
    size_t total = 0;
    for ( size_t i = 0; i < items.size(); ++ i ) {
        total += items[ i ].size();
    }; // for i
    write_item( stream, total + ( use_eors ? eor.size() * items.size() : 0 ) );
    for ( size_t i = 0; i < items.size(); ++ i ) {
        items[ i ].write( stream );
        if ( use_eors ) {
            eor.write( stream );
        }; // if
    }; // for i
};


template<
    typename item_t
>
void
read_packet(
    std::istream &                  stream,
    std::vector< item_t > &         items,
    bool                            use_eors = true
) {
    logger_t _logger( "TMP" );
    items.clear();
    size_t size = 0;
    read_item( stream, size );
    ERR(
        size % alignment != 0,
        "Packet size (" << size << ") is not aligned to " << alignment
    );
    std::ostream::pos_type start = stream.tellg();
    while ( size_t( stream.tellg() - start ) < size ) {
        item_t item;
        item.read( stream );
        if ( use_eors ) {
            eor.read( stream );
        }; // if
        items.push_back( item );
    };
    ERR( size_t( stream.tellg() - start ) != size, "OOPS" );
};


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#endif // _DATA_TPP_

// end of file //
