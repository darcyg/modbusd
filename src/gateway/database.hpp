// database.hpp //

/**
    @file
    @brief Database interface.
*/

#ifndef _DATABASE_HPP_
#define _DATABASE_HPP_


#include "string.hpp"
#include "error.hpp"
#include "logger.hpp"
#include "command.hpp"
#include "object.hpp"

#include <sqlite3.h>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


class db_err_t : public err_t {
    public :
        db_err_t(
            char const *        file,
            int                 line,
            string_t const &    prefix,
            string_t const &    message,
            string_t const &    function,
            int                 code
        );
        int code() const;
    private :
        int _code;
}; // class db_err_t


#define SQLERR( _action_, _func_, _code_ ) {            \
    if ( _code_ != SQLITE_OK ) { \
        db_err_t _err_( __FILE__, __LINE__, _logger.prefix(), _action_, _func_, _code_ );  \
        _logger._log( _SOURCE_LOCATION_, logger_t::error, _err_.msg() );                            \
        throw _err_;                                       \
    };                                                  \
}

#define SQLWRN( func, code ) {                          \
    if ( code != SQLITE_OK ) {                          \
        db_err_t ex( __FILE__, __LINE__, "", func, code );  \
        WLOG( ex );                                     \
    };                                                  \
}


#define _SQLCALL( _ex_, _action_, _func_, _stmt_, _success_, _result_ ) {        \
    DLOG( _action_ << "..." );                                            \
    _stmt_;                                                               \
    if ( ! ( _success_ ) ) {                                              \
        osstream_t _stream_;                                            \
        _stream_ << _action_;                   \
        db_err_t _err_( _SOURCE_LOCATION_, _logger.prefix(), _stream_.str(), _func_, error );      \
        if ( _ex_ ) {                                                  \
            throw _err_;                                                 \
        };                                                              \
    };                                                                  \
    DLOG( _result_ );                                                     \
}

#define ESQLCALL( action, func, call, success, result )                 \
    _SQLCALL( 1, action, func, call, success, result )

#define WSQLCALL( action, func, call, success, result )                 \
    _SQLCALL( 0, action, func, call, success, result )


class database_t : public object_t {

    protected :

        class db_t : public object_t {
            public :
                db_t(
                    string_t const & name,
                    string_t const & path,
                    int              timeout
                 );
                ~db_t();
            public :
                string_t    _path;
                int         _timeout;
                sqlite3 *   _db;
        }; // db_t

        class stmt_t : public object_t {
            public :
                stmt_t( string_t const & name, string_t const & text, db_t & db );
                ~stmt_t();
                void reset();
                int  step( int const * expected = NULL );
            public :
                sqlite3_stmt *  _stmt;
        }; // class _stme_t

    public :

        database_t(
            string_t const & name,      ///< Database name, used in log messages only.
            string_t const & path,      ///< Database path, location of database's .sdb file.
            int              timeout    ///< Database timeout.
        );

    protected :

        void _begin();
        void _commit();

    public :

        db_t        _db;
        stmt_t      _begin_stmt;
        stmt_t      _commit_stmt;

}; // class database_t


template<
    typename item_t
>
struct db_traits_t {

    /**
        @brief Text of @c SELECT statement.
    */
    static string_t select_statement();

    /**
        @brief Text of @c INSERT statement.
    */
    static string_t insert_statement();

    static item_t get( logger_t & _logger, database_t::stmt_t * stmt );

    static void   put( logger_t & _logger, database_t::stmt_t * stmt, item_t const & item );

}; // struct db_traits_t


template<
    typename item_t
>
class _database_t : public database_t {

    public :

        typedef std::vector< item_t > items_t;

        explicit _database_t(
            string_t const & name,
            string_t const & path,
            int              timeout
        );

        items_t select();
        void    insert( items_t const & items );

    protected :

        stmt_t _select_stmt;
        stmt_t _insert_stmt;

}; // class database_t


typedef _database_t< sample_t >     sample_database_t;
typedef _database_t< event_t >      event_database_t;
typedef _database_t< setting_t >    setting_database_t;


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#include "database.tpp"


#endif // _DATABASE_HPP_

// end of file //
