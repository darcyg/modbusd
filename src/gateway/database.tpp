// database.tpp //

/**
    @file
    @brief Database template implementation.
*/

#ifndef _DATABASE_TPP_
#define _DATABASE_TPP_

#include "database.hpp"

#include "system.hpp"

#include <cassert>

namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


/**
    @brief Small adapter.

    @c sqlite_3_text returns a pointer to @c unsigned @c char, and may return @c NULL, so
    constructing a string from result of @c sqlite3_text is not trivial.
*/
static
inline
string_t
_sqlite3_column_str(
    sqlite3_stmt *  stmt,
    int             col
) {
    unsigned char const * str = sqlite3_column_text( stmt, col );
    return string_t( str == NULL ? "" : reinterpret_cast< char const * >( str ) );
};


template<
>
struct db_traits_t< sample_t > {
    typedef sample_t item_t;
    static string_t select_statement() {
        return
            "SELECT `ParamId`, `ChannelId`, MAX( `RegisterDate` ), `Value` "
            "FROM `tblparamdata` "
            "GROUP BY `ChannelId`";
    };
    static string_t insert_statement() {
        return
            "INSERT INTO `tblparamdata` "
                "( `ParamId`, `ChannelId`, `RegisterDate`, `Value` ) "
            "VALUES "
                "( ?1, ?2, ?3, ?4 )";
    };
    static item_t get( logger_t & _logger, sqlite3_stmt * stmt ) {
        return item_t(
            int(       sqlite3_column_int(    stmt, 0 ) ),
            channel_t(  sqlite3_column_int(    stmt, 1 ) ),
            date_t(    _sqlite3_column_str(    stmt, 2 ) ),
            value_t(    sqlite3_column_double( stmt, 3 ) )
        );
    };
    static void put( logger_t & _logger, sqlite3_stmt * stmt, item_t const & item ) {
        int error;
        error = sqlite3_bind_int( stmt, 1, item.id );
        SQLERR( "TODO", "sqlite3_bind_int", error );
        error = sqlite3_bind_int( stmt, 2, item.channel.bulk );
        SQLERR( "TODO", "sqlite3_bind_int", error );
        error = sqlite3_bind_text( stmt, 3, item.date.bulk, -1, NULL );
        SQLERR( "TODO", "sqlite3_bind_text", error );
        error = sqlite3_bind_double( stmt, 4, item.value );
        SQLERR( "TODO", "sqlite3_bind_double", error );
    };
}; // struct db_traits_t< sample_t >


template<
>
struct db_traits_t< event_t > {
    typedef event_t item_t;
    static string_t select_statement() {
        return
            "SELECT "
                "`TypeId`, `ChannelId`, MAX( `RegisterDate` ), "
                "`Argument1`, `Argument2`, `Argument3`, `Argument4` "
            "FROM `tbleventbus` "
            "WHERE `TypeId` <> " + str( setting_t::set_id ) + " "
            "GROUP BY `ChannelId`";
    };
    static string_t insert_statement() {
        return
            "INSERT INTO `tbleventbus`"
                "( `TypeId`, `ChannelId`, `RegisterDate`, "
                "`Argument1`, `Argument2`, `Argument3`, `Argument4` ) "
            "VALUES "
                "( ?1, ?2, ?3, ?4, ?5, ?6, ?7 )";
    };
    static item_t get( logger_t & _logger, sqlite3_stmt * stmt ) {
        return item_t(
            int(       sqlite3_column_int( stmt, 0 ) ),
            channel_t(  sqlite3_column_int( stmt, 1 ) ),
            date_t(    _sqlite3_column_str( stmt, 2 ) ),
            string_t(  _sqlite3_column_str( stmt, 3 ) ),
            string_t(  _sqlite3_column_str( stmt, 4 ) ),
            string_t(  _sqlite3_column_str( stmt, 5 ) ),
            string_t(  _sqlite3_column_str( stmt, 6 ) )
        );
    };
    static void put( logger_t & _logger, sqlite3_stmt * stmt, event_t const & item ) {
        int error;
        error = sqlite3_bind_int( stmt, 1, item.id );
        SQLERR( "TODO", "sqlite3_bind_int", error );
        error = sqlite3_bind_int( stmt, 2, item.channel.bulk );
        SQLERR( "TODO", "sqlite3_bind_int", error );
        error = sqlite3_bind_text( stmt, 3, item.date.bulk, -1, NULL );
        SQLERR( "TODO", "sqlite3_bind_text", error );
        error = sqlite3_bind_text( stmt, 4, item.arg1.c_str(), -1, NULL );
        SQLERR( "TODO", "sqlite3_bind_text", error );
        error = sqlite3_bind_text( stmt, 5, item.arg2.c_str(), -1, NULL );
        SQLERR( "TODO", "sqlite3_bind_text", error );
        error = sqlite3_bind_text( stmt, 6, item.arg3.c_str(), -1, NULL );
        SQLERR( "TODO", "sqlite3_bind_text", error );
        error = sqlite3_bind_text( stmt, 7, item.arg4.c_str(), -1, NULL );
        SQLERR( "TODO", "sqlite3_bind_text", error );
    };
}; // struct bd_traits_t< event_t >


template<
>
struct db_traits_t< setting_t > : public db_traits_t< event_t > {
    typedef setting_t item_t;
    static string_t select_statement() {
        return
            "SELECT "
                "`TypeId`, `ChannelId`, MAX( `RegisterDate` ), "
                "`Argument1`, `Argument2`, `Argument3`, `Argument4` "
            "FROM `tbleventbus` "
            "WHERE `TypeId` = " + str( setting_t::set_id ) + " "
            "GROUP BY `Argument3`";
    };
}; // struct bd_traits_t< settings_t >


template<
    typename item_t
>
_database_t< item_t >::_database_t(
    string_t const & name,
    string_t const & path,
    int              timeout
) :
    database_t( name, path, timeout ),
    _select_stmt( "SELECT", db_traits_t< item_t >::select_statement(), _db ),
    _insert_stmt( "INSERT", db_traits_t< item_t >::insert_statement(), _db )
{
};


template<
    typename item_t
>
typename _database_t< item_t >::items_t
_database_t< item_t >::select(
) {
    items_t items;
    DLOG( "selecting rows..." );
    _select_stmt.reset();
    for ( ; ; ) {
        static int const expected[] = { SQLITE_ROW, SQLITE_DONE, SQLITE_OK };
        int error = _select_stmt.step( expected );
        if ( error == SQLITE_ROW ) {
            item_t item(  db_traits_t< item_t >::get( _logger, _select_stmt._stmt ) );
            DLOG( "row selected: " << item );
            items.push_back( item );
        } else if ( error == SQLITE_DONE ) {
            break;
        } else {
            SQLERR( "selecting rows", "sqlite3_step", error );
        }; // if
    }; // forever
    DLOG( items.size() << " row(s) selected" );
    return items;
};


template<
    typename item_t
>
void
_database_t< item_t >::insert(
    items_t const & items
) {
    if ( items.empty() ) {
        DLOG( "nothing to insert" );
    } else {
        DLOG( "inserting " << items.size() << " row(s)..." );
        timer_t timer;
        timer.start();
        _begin();
        for ( size_t i = 0; i < items.size(); ++ i ) {
            _insert_stmt.reset();
            db_traits_t< item_t >::put( _logger, _insert_stmt._stmt, items[ i ] );
            _insert_stmt.step();
        }; // for i
        _commit();
        timer.stop();
        DLOG( items.size() << " row(s) inserted in << " << timer );
    }; // if
};


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#endif // _DATABASE_HPP_

// end of file //
