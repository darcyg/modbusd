// database.cpp //

/**
    @file
    @brief Database non-template implementation.
*/

#include "database.hpp"

#include "system.hpp"

#include <cassert>

#include <algorithm>

#include <unistd.h>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


db_err_t::db_err_t(
    char const *        file,
    int                 line,
    string_t const &    prefix,
    string_t const &    message,
    string_t const &    function,
    int                 code
) :
    // `sqlite3_errstr' is described in the manual, but is not available on my system,
    // so I cannot convert SQLite3 error code into human-friendly message...
    err_t( file, line, prefix, message + ": `" + function + "' failed: Error " + str( code ) ),
    _code( code )
{
};


int
db_err_t::code(
) const {
    return _code;
}; // code


static
void
db_logger(
    void *          data,
    int             code,
    const char *    message
) {
    static logger_t _logger( "sqlite3" );
    ELOG( message );
};


// -------------------------------------------------------------------------------------------------
// database_t::db_t class
// -------------------------------------------------------------------------------------------------


database_t::db_t::db_t(
    string_t const & name,
    string_t const & path,
    int              timeout
) :
    object_t( name + " db" ),
    _path( path ),
    _timeout( timeout )
{
    int error = 0;

    #if defined( SQLITE_CONFIG_LOG )
        // SQLite on target platform does not have SQLITE_CONFIG_LOG.

        // sqlite3 should be configured only once.
        // Cannot use `mutex_t' here because we need statically initialized mutex.
        static pthread_mutex_t mutex  = PTHREAD_MUTEX_INITIALIZER;
        static bool volatile   inited = false;
        static string_t const  action = "configuring sqlite3";

        if ( ! inited ) {

            EPTHCALL(
                "locking sqlite configuration mutex",
                "pthread_mutex_lock",
                pthread_mutex_lock( & mutex ),
                "sqlite configuration mutex locked"
            );

            if ( ! inited ) {
                WSQLCALL(
                    "configuring sqlite3",
                    "sqlite3_config",
                    error = sqlite3_config( SQLITE_CONFIG_LOG, db_logger, NULL ),
                    ! error,
                    "sqlite3 configured"
                );
            }; // if

            inited = true;

            EPTHCALL(
                "unlocking sqlite configuration mutex",
                "pthread_mutex_unlock",
                pthread_mutex_unlock( & mutex ),
                "sqlite configuration mutex unlocked"
            );

        }; // if
    #endif

    ILOG( "Database path = `" << _path << "'" );
    ILOG( "Timeout = " << _timeout << " s" );

    ESQLCALL(
        "opening",
        "sqlite3_open_v2",
        error = sqlite3_open_v2( _path.c_str(), & _db, SQLITE_OPEN_READWRITE, NULL ),
        ! error,
        "opened"
    );

    ERR(
        _timeout < 0 || _timeout > INT_MAX / 1000,
        "Database timeout " << _timeout << " is out of range 0 .. " << INT_MAX / 1000
    );
    ESQLCALL(
        "setting timeout to " << _timeout << " s",
        "sqlite3_busy_timeout",
        error = sqlite3_busy_timeout( _db, _timeout * 1000 ),
        ! error,
        "timeout set"
    );

};


database_t::db_t::~db_t(
) {
    int error;
    WSQLCALL(
        "closing",
        "sqlite3_close",
        error = sqlite3_close( _db ),
        ! error,
        "closed"
    );
};


// -------------------------------------------------------------------------------------------------
// database_t::stmt_t class
// -------------------------------------------------------------------------------------------------


database_t::stmt_t::stmt_t(
    string_t const &    name,
    string_t const &    text,
    db_t &              db
) :
    object_t( db, name + " stmt" ),
    _stmt( NULL )
{
    ILOG( "Statement text = `" << text << "'" );
    //~ _logger.verbose( false );
    int error = 0;
    ESQLCALL(
        "preparing",
        "sqlite3_prepare_v2",
        error = sqlite3_prepare_v2( db._db, text.c_str(), -1, & _stmt, NULL ),
        ! error,
        "prepared"
    );
};


database_t::stmt_t::~stmt_t(
) {
    if ( _stmt ) {
        int error = 0;
        WSQLCALL(
            "finalizing",
            "sqlite3_finalize",
            error = sqlite3_finalize( _stmt ),
            ! error,
            "finalized"
        );
        _stmt = NULL;
    }; // if
};


void
database_t::stmt_t::reset(
) {
    int error = 0;
    ESQLCALL(
        "resetting",
        "sqlite3_reset",
        error = sqlite3_reset( _stmt ),
        ! error,
        "reset"
    );
}; // reset


int
database_t::stmt_t::step(
    int const *    expected
) {
    static int const _expected[] = { SQLITE_ROW, SQLITE_DONE, SQLITE_OK };
    if ( expected == NULL ) {
        expected = _expected;
    }; // if
    int error = 0;
    ESQLCALL(
        "stepping",
        "sqlite3_step",
        int e = error = sqlite3_step( _stmt );
        for ( size_t i = 0; expected[ i ] != SQLITE_OK; ++ i ) {
            if ( error == expected[ i ] ) {
                e = SQLITE_OK;
                break;
            }; // if
        },
        e == SQLITE_OK,
        "stepped"
    );
    return error;
}; // _step


// -------------------------------------------------------------------------------------------------
// database_t::stmt_t class
// -------------------------------------------------------------------------------------------------


database_t::database_t(
    string_t const & name,
    string_t const & path,
    int              timeout
) :
    object_t( name + " database" ),
    _db( name, path, timeout ),
    _begin_stmt( "BEGIN", "BEGIN", _db ),
    _commit_stmt( "COMMIT", "COMMIT", _db )
{
    //~ _logger.verbose( false );
};


void
database_t::_begin(
) {
    DLOG( "begin transaction..." );
    _begin_stmt.reset();
    #if 1
        // Original variant. Sometimes with error SQLITE_BUSY.
        // Probably, setting timeout helps.
        _begin_stmt.step();
    #else
        // New variant, probably incorrect, because SQLITE_BUSY may be returned either from
        // `INSERT' or from `COMMIT'.
        static int const expected[] = { SQLITE_BUSY, SQLITE_OK };
        for ( ; ; ) {
            int status = _begin_stmt.step( expected );
            if ( status == SQLITE_OK ) {
                break;
            };
            DLOG( "database busy, waiting a second..." );
            sleep( 1 );
        }; // forever
    #endif
};


void
database_t::_commit(
) {
    DLOG( "end transaction..." );
    _commit_stmt.reset();
    _commit_stmt.step();
};


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


// end of file //
