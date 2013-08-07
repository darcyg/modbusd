// mutex.cpp //

/**
    @file
    @brief Mutex implementation.
*/

#include "mutex.hpp"
#include "error.hpp"


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


// -------------------------------------------------------------------------------------------------
// mutex_t::attr_t class
// -------------------------------------------------------------------------------------------------


static logger_t mutex_attr_logger( "mutex attr" );


mutex_t::attr_t::attr_t(
    int type
) {
    int error = 0;
    logger_t & _logger = mutex_attr_logger;
    error = pthread_mutexattr_init( & _attr );
    PTHERR( "Initializing", "pthread_init_attr", error );
    error = pthread_mutexattr_settype( & _attr, type );
    PTHERR( "Setting type", "pthread_mutexattr_attr", error );
}; // ctor


mutex_t::attr_t::~attr_t(
) {
    logger_t & _logger = mutex_attr_logger;
    int error = pthread_mutexattr_destroy( & _attr );
    PTHWRN( "Destroying", "pthread_destroy_attr", error );
}; // dtor


// -------------------------------------------------------------------------------------------------
// mutex_t::locker_t class
// -------------------------------------------------------------------------------------------------


mutex_t::locker_t::locker_t(
    mutex_t &  mutex
) :
    _mutex( mutex )
{
    mutex.lock();
}; // ctor


mutex_t::locker_t::~locker_t(
) {
    _mutex.unlock();
}; // dtor


// -------------------------------------------------------------------------------------------------
// mutex_t class
// -------------------------------------------------------------------------------------------------


static logger_t mutex_logger( "mutex" );


mutex_t::mutex_t(
    int              type
) {
    logger_t & _logger = mutex_logger;
    attr_t attr( type );
    int error = pthread_mutex_init( & _mutex, & attr._attr );
    PTHERR( "Initializing", "pthread_mutex_init", error );
};


mutex_t::~mutex_t(
) {
    logger_t & _logger = mutex_logger;
    int error = pthread_mutex_destroy( & _mutex );
    PTHWRN( "Destroying", "pthread_mutex_destroy", error );
};


void
mutex_t::lock(
) {
    logger_t & _logger = mutex_logger;
    int error = pthread_mutex_lock( & _mutex );
    PTHERR( "Locking", "pthread_mutex_lock", error );
};


void
mutex_t::unlock(
) {
    logger_t & _logger = mutex_logger;
    int error = pthread_mutex_unlock( & _mutex );
    PTHERR( "Unlocking", "pthread_mutex_unlock", error );
};


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


// end of file //
