// data.cpp //

/**
    @file
    @brief Implementation of non-template functions declared in data.hpp
*/

#include "data.hpp"

#include "string.hpp"
#include "error.hpp"

#include <cassert>
#include <climits>      // INT_MAX
#include <cstring>      // memset

#include <arpa/inet.h>  // htonl, ntohl


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


static logger_t _logger( "data" );


// -------------------------------------------------------------------------------------------------
// data items (numbers and strings).
// -------------------------------------------------------------------------------------------------


static inline
size_t
str_pad(
    string_t const & item
) {
    size_t pad = ( alignment - ( item.size() % alignment ) ) % alignment;
    assert( pad < alignment );
    assert( ( item.size() + pad ) % alignment == 0 );
    return pad;
};


/**
    @brief Number of bytes required for a string (counting string padding and string length).

    When transmitting a string through a socket, we transmit aligned string length first, then
    padded string. This function returns total number of bytes to be transmitted.
*/
static inline
size_t
str_size(
    string_t const & item
) {
    return sizeof( int ) + item.size() + str_pad( item );
};


/**
    @brief Write integer value into a stream.

    Integer value is written in binary form, using network byte order.
*/
void
write_item(
    std::ostream &  stream,
    int const &     item
) {
    STATIC_ASSERT( sizeof( item ) == alignment );
    int i = htonl( item );
    stream.write( reinterpret_cast< char const * >( & i ), sizeof( i ) );
};


void
write_item(
    std::ostream &  stream,
    float const &   item
) {
    STATIC_ASSERT( sizeof( item ) == sizeof( int ) );
    write_item( stream, reinterpret_cast< int const & >( item ) );
};


void
write_item(
    std::ostream &  stream,
    size_t const &  item
) {
    ERR(
        item > INT_MAX,
        "Attemp to write too large size (" << item << ") "
            << "max allowed size is " << INT_MAX
    );
    write_item( stream, int( item ) );
};


/**
    @brief Read an integer value from a stream.

    The function takes care about byte order: it converts value from network to host
    representation.
*/
void
read_item(
    std::istream &  stream,
    int &           item
) {
    STATIC_ASSERT( sizeof( item ) == alignment );
    int i;
    stream.read( reinterpret_cast< char * >( & i ), sizeof( i ) );
    item = ntohl( i );
};


/**
    @brief Read a size from a stream.

    The same as reading integer, but additionally checks the read value is not negative.
*/
void
read_item(
    std::istream &  stream,
    size_t &        item
) {
    int i;
    read_item( stream, i );
    ERR(
        i < 0,
        "Negative size (" << i << ") read"
    );
    item = i;
};


void
read_item(
    std::istream &  stream,
    float &         item
) {
    //~ int i;
    read_item( stream, reinterpret_cast< int & >( item ) );
    //~ item = reinterpret_cast< float & >( i );
};


// -------------------------------------------------------------------------------------------------
// variable-size strings.
// -------------------------------------------------------------------------------------------------


vstring_t::vstring_t(
) {
};


vstring_t::vstring_t(
    string_t const & str
) :
    bulk( str )
{
};


vstring_t::vstring_t(
    char const * str
) :
    bulk( str )
{
};


char const *
vstring_t::data(
) const {
    return bulk.data();
};


string_t
vstring_t::str(
) const {
    return bulk;
};


char const *
vstring_t::c_str(
) const {
    return bulk.c_str();
};


bool
vstring_t::operator ==(
    vstring_t const & rhs
) const {
    return bulk == rhs.bulk;
};


bool
vstring_t::operator !=(
    vstring_t const & rhs
) const {
    return ! ( * this == rhs );
};


bool
vstring_t::operator <(
    vstring_t const & rhs
) const {
    return ( bulk < rhs.bulk );
};


/**
    @brief Number of characters to append to a string to make its length aligned.
*/
size_t
vstring_t::pad(
) const {
    size_t pad = ( alignment - ( bulk.size() % alignment ) ) % alignment;
    assert( pad < alignment );
    assert( ( bulk.size() + pad ) % alignment == 0 );
    return pad;
};


/**
    @brief Size of string as data item.

    When transmitting a variable-length string through a socket, we transmit aligned string length
    first, then padded string. This function returns total number of bytes to be transmitted.

    To get number of characters in the string, use @c str().size().
*/
size_t
vstring_t::size(
) const {
    return sizeof( int ) + bulk.size() + pad();
};


/**
    @brief Writes a variable-length string to a stream.

    String is written as two pieces: first goes string's length, then string characters. String's
    length is rounded up be be properly aligned, string body is padded with zero characters up to
    aligned length.
*/
void
vstring_t::write(
    std::ostream & stream
) const {
    static string_t const zero( alignment, '\x00' );
    size_t p = pad();
    assert( ( bulk.size() + p ) % alignment == 0 );
    write_item( stream, bulk.size() + p );
    stream.write( bulk.data(), bulk.size() );
    stream.write( zero.data(), p );
};


/**
    @brief Read a variable-length string from a stream.

    The function reads string length first, then reads string characters. Padding zero characters
    are truncated.
*/
void
vstring_t::read(
    std::istream & stream
) {
    size_t size = 0;
    read_item( stream, size );
    // It would be nice to resize string and read directly into the string, but unfortunately
    // string class does not allow it: string's `data()' returns ponter to constant characters.
    // So we have to use a buffer and extra copy.
    buffer_t buffer( size );
    stream.read( buffer.data(), buffer.size() );
    // Drop padding (trailing zero characters).
    while ( ! buffer.empty() && buffer.back() == 0 ) {
        buffer.pop_back();
    }; // while
    bulk.assign( buffer.data(), buffer.size() );
};


std::ostream &
operator <<(
    std::ostream &          stream,
    vstring_t const &       str
) {
    stream << c_literal( str.str() );
    return stream;
};


// -------------------------------------------------------------------------------------------------
// data structures: samples, events, settings.
// -------------------------------------------------------------------------------------------------


/**
    @brief Default constructor.

    Fills all files with zeros.
*/
sample_t::sample_t(
) :
    id( 0 ),
    value( 0.0 )
{
};

sample_t::sample_t(
    int        _id,
    channel_t   _ch,
    date_t      _dt,
    value_t     _vl
) :
    id( _id ),
    channel( _ch ),
    date( _dt ),
    value( _vl )
{
};


bool
sample_t::operator ==(
    sample_t const & rhs
) const {
    return id == rhs.id && channel == rhs.channel && date == rhs.date && value == rhs.value;
};


sample_t::key_t
sample_t::key(
) const {
    return channel;
};


size_t
sample_t::size(
) const {
    return sizeof( id ) + sizeof( channel ) + date.size() + sizeof( value );
};


void
sample_t::write(
    std::ostream &   stream
) const {
    write_item( stream, id      );
    channel.write( stream );
    date.write( stream );
    write_item( stream, value   );
};


void
sample_t::read(
    std::istream &  stream
) {
    read_item( stream, id );
    channel.read( stream );
    date.read( stream );
    read_item( stream, value );
};


std::ostream &
operator <<(
    std::ostream &      stream,
    sample_t const &    sample
) {
    stream
        << "{ "
        << "id="      << sample.id      << ", "
        << "channel=" << sample.channel << ", "
        << "date="    << sample.date    << ", "
        << "value="   << sample.value
        << " }";
    return stream;
}; // <<


int const _event_t::set_id = 11;

_event_t::_event_t(
) :
    id( 0 )
{
};


_event_t::_event_t(
    int             _id,
    channel_t        _ch,
    date_t           _dt,
    string_t const & _a1,
    string_t const & _a2,
    string_t const & _a3,
    string_t const & _a4
) :
    id( _id ),
    channel( _ch ),
    date( _dt ),
    arg1( _a1 ),
    arg2( _a2 ),
    arg3( _a3 ),
    arg4( _a4 )
{
};


bool
_event_t::operator ==(
    _event_t const & rhs
) const {
    return
        id == rhs.id && channel == rhs.channel && date == rhs.date
        && arg1 == rhs.arg1 && arg2 == rhs.arg2 && arg3 == rhs.arg3 && arg4 == rhs.arg4;
};


size_t
_event_t::size(
) const {
    return
        sizeof( id ) + sizeof( channel ) + date.size()
        + arg1.size() + arg2.size() + arg3.size() + arg4.size();
};


void
_event_t::write(
    std::ostream &   stream
) const {
    write_item( stream, id      );
    channel.write( stream );
    date.write( stream );
    arg1.write( stream );
    arg2.write( stream );
    arg3.write( stream );
    arg4.write( stream );
};


void
_event_t::read(
    std::istream &  stream
) {
    read_item( stream, id );
    channel.read( stream );
    date.read( stream );
    arg1.read( stream );
    arg2.read( stream );
    arg3.read( stream );
    arg4.read( stream );
};


std::ostream &
operator <<(
    std::ostream &      stream,
    _event_t const &    event
) {
    stream
        << "{ "
        << "id="      << event.id              << ", "
        << "channel=" << event.channel         << ", "
        << "date="    << event.date            << ", "
        << "arg1="    << c_literal( event.arg1.str() ) << ", "
        << "arg2="    << c_literal( event.arg2.str() ) << ", "
        << "arg3="    << c_literal( event.arg3.str() ) << ", "
        << "arg4="    << c_literal( event.arg4.str() )
        << " }";
    return stream;
}; // <<


event_t::event_t(
) {
};


event_t::event_t(
    _event_t const & event
) :
    _event_t( event )
{
    //~ ERR(
        //~ id == _event_t::set_id,
        //~ "Invalid event id " << id << ", must not be " << set_id
    //~ );
};


event_t::event_t(
    int             _id,
    channel_t        _ch,
    date_t           _dt,
    string_t const & _a1,
    string_t const & _a2,
    string_t const & _a3,
    string_t const & _a4
) :
    _event_t( _id, _ch, _dt, _a1, _a2, _a3, _a4 )
{
    //~ ERR(
        //~ id == _event_t::set_id,
        //~ "Invalid event id " << id << ", must not be " << set_id
    //~ );
};


event_t::key_t
event_t::key(
) const {
    return channel;
};


setting_t::setting_t(
) {
};


setting_t::setting_t(
    _event_t const & event
) :
    _event_t( event )
{
    ERR(
        id != set_id,
        "Invalid setting id " << id << ", must be " << set_id
    );
};


setting_t::setting_t(
    int             _id,
    channel_t        _ch,
    date_t           _dt,
    string_t const & _a1,
    string_t const & _a2,
    string_t const & _a3,
    string_t const & _a4
) :
    _event_t( _id, _ch, _dt, _a1, _a2, _a3, _a4 )
{
    ERR(
        id != set_id,
        "Invalid setting id " << id << ", must be " << set_id
    );
};


setting_t::key_t
setting_t::key(
) const {
    return arg3.str();
};


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


// end of file //
