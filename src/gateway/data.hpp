// data.hpp //

/**
    @file
    @brief Elementary data types and basic structures.
*/

#ifndef _DATA_HPP_
#define _DATA_HPP_


#include "string.hpp"

#include "error.hpp"

#include <istream>
#include <ostream>
#include <vector>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {

/**
    @brief Alignment. We will align layout of structures using this constant.
*/
size_t const alignment = sizeof( int );


// -------------------------------------------------------------------------------------------------
// data items (numbers and strings).
// -------------------------------------------------------------------------------------------------


/**
    @brief Identifier of a sample or event.
*/
//typedef int id_t;


/**
    @brief Sapmple's value.
*/
typedef float   value_t;


// Functions for sending various data items through socket.
void write_item( std::ostream & stream, int      const & item );
void write_item( std::ostream & stream, size_t   const & item );
void write_item( std::ostream & stream, float    const & item );
void write_item( std::ostream & stream, string_t const & item );

void read_item( std::istream & stream, int        & item  );
void read_item( std::istream & stream, size_t     & item  );
void read_item( std::istream & stream, float      & item  );
void read_item( std::istream & stream, string_t   & item  );

// -------------------------------------------------------------------------------------------------
// channel
// -------------------------------------------------------------------------------------------------

/**
    @brief Channel id.
*/

struct channel_t {
    friend std::ostream & operator <<( std::ostream & stream, channel_t const & ch ) {
        stream << ch.bulk;
        return stream;
    };
    explicit channel_t( int id = 0 ) : bulk( id ) {};
    bool operator ==( channel_t const & rhs ) const { return bulk == rhs.bulk; };
    bool operator <( channel_t const & rhs ) const { return bulk < rhs.bulk; };
    size_t size() const { return sizeof( bulk ); };
    void write( std::ostream & stream ) const { write_item( stream, bulk ); };
    void read( std::istream & stream ) { read_item( stream, bulk ); };
    int bulk;
}; // struct channel_t

typedef std::vector< channel_t > channels_t;


// -------------------------------------------------------------------------------------------------
// fixed-size strings.
// -------------------------------------------------------------------------------------------------

/**
    @brief Fixed-length string.

    Fixed-size strings are used for representing such datatypes as dates (20 characters) or marks
    (4 characters).

    Size must be aligned.
*/
template<
    size_t n
>
class fstring_t {
    public :
        fstring_t();
        explicit fstring_t( string_t const & str );
        char const * data() const;
        string_t     str()  const;
        bool operator ==( fstring_t< n > const & rhs ) const;
        bool operator !=( fstring_t< n > const & rhs ) const;
        bool operator >(  fstring_t< n > const & rhs ) const;
        size_t size() const;
        void   write( std::ostream & stream ) const;
        void   read( std::istream & stream );
        void   read( std::istream & stream ) const;
    public :
        char bulk[ n ];
}; // struct fstring_t


template< size_t n >
std::ostream & operator <<( std::ostream & stream, fstring_t< n > const & item );


/**
    @brief 4-character string.

    Used in the beginning of command and in replies.
*/
typedef fstring_t< 4 > mark_t;


/**
    @brief Record terminator.

    "eor" stands for "end of record". Record terminator finished each record in a packet. (However,
    not all the packets use it.)
*/
static mark_t const eor( "$\x80\x80\x80" );


/**
    @brief 20-character string.

    Used for dates. Date itself occupies 19 characters (yyy-mm-dd hh:mm:ss) plus zero character.
    Zero character aligns the total size, and allows using a date as C zero-terminated string.
*/
typedef fstring_t< 20 > date_t;


// -------------------------------------------------------------------------------------------------
// variasble-size strings.
// -------------------------------------------------------------------------------------------------

/**
    @brief Variable-length string.
*/
class vstring_t {
    public :
        vstring_t();
        vstring_t( string_t const & str );
        vstring_t( char const * str );
        char const * data() const;
        string_t     str()  const;
        char const * c_str() const;
        bool operator ==( vstring_t const & rhs ) const;
        bool operator !=( vstring_t const & rhs ) const;
        bool operator <( vstring_t const & rhs ) const;
        size_t pad() const;
        size_t size() const;
        void   write( std::ostream & stream ) const;
        void   read( std::istream & stream );
    private :
        string_t bulk;
}; // class vstring_t


std::ostream & operator <<( std::ostream & stream, vstring_t const & str );

typedef std::vector< vstring_t > vstrings_t;


// -------------------------------------------------------------------------------------------------
// data structures: sampples, events, settings.
// -------------------------------------------------------------------------------------------------


/**
    @brief Result of measurement.
*/
struct sample_t {

    typedef channel_t key_t;

    sample_t();
    sample_t(
        int        _id,
        channel_t   _ch,
        date_t      _dt,
        value_t     _vl
    );
    bool operator ==( sample_t const & rhs ) const;
    key_t  key()  const;
    size_t size() const;
    void write( std::ostream & stream ) const;
    void read( std::istream & stream );

    int        id;         ///< Parameter id.
    channel_t   channel;    ///< Channel id.
    date_t      date;       ///< Date of measurement.
    value_t     value;      ///< Parameter Value.

}; // struct sample_t


std::ostream & operator <<( std::ostream & stream, sample_t const & sample );


typedef std::vector< sample_t  > samples_t;

/**
    @brief Base class for event and setting.

    Events and settings are almost the same, but use different keys.
*/
struct _event_t {

    static int const set_id;

    _event_t();
    _event_t(
        int             _id,
        channel_t        _ch,
        date_t           _dt,
        string_t const & _a1 = "",
        string_t const & _a2 = "",
        string_t const & _a3 = "",
        string_t const & _a4 = ""
    );
    bool operator ==( _event_t const & rhs ) const;
    size_t size() const;
    void write( std::ostream & stream ) const;
    void read( std::istream & stream );

    int        id;         ///< Event type id.
    channel_t   channel;    ///< Channel id.
    date_t      date;       ///< Event date.
    vstring_t   arg1;
    vstring_t   arg2;
    vstring_t   arg3;
    vstring_t   arg4;

}; // struct _event_t


typedef std::vector< _event_t   > _events_t;


std::ostream & operator <<( std::ostream & stream, _event_t const & event  );


/**
    @brief An event, not setting.
*/
struct event_t : public _event_t {

    typedef channel_t key_t;

    event_t();
    event_t( _event_t const & ev );
    event_t(
        int             _id,
        channel_t        _ch,
        date_t           _dt,
        string_t const & _a1 = "",
        string_t const & _a2 = "",
        string_t const & _a3 = "",
        string_t const & _a4 = ""
    );
    key_t  key()  const;

}; // struct event_t


typedef std::vector< event_t   > events_t;


/**
    @brief A setting.
*/
struct setting_t : public _event_t {

    typedef vstring_t key_t;

    setting_t();
    setting_t( _event_t const & ev );
    setting_t(
        int             _id,
        channel_t        _ch,
        date_t           _dt,
        string_t const & _a1 = "",
        string_t const & _a2 = "",
        string_t const & _a3 = "",
        string_t const & _a4 = ""
    );
    key_t  key()  const;

}; // struct setting_t


typedef std::vector< setting_t > settings_t;


template<
    typename item_t
>
void
write_packet(
    std::ostream &                  stream,
    std::vector< item_t > const &   items,
    bool                            use_eors = true

);


template<
    typename item_t
>
void
read_packet(
    std::istream &                  stream,
    std::vector< item_t > &         items,
    bool                            use_eors = true
);


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#include "data.tpp"


#endif // _DATA_HPP_

// end of file //
