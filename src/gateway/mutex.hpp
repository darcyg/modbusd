// mutex.hpp //

/**
    @file
    @brief Mutex interface.
*/

#ifndef _MUTEX_HPP_
#define _MUTEX_HPP_


#include "object.hpp"

#include <pthread.h>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {

/**
    @brief Mutex class.
*/
class mutex_t {

    public :

        /**
            @brief Mutex locker.

            Frankly, this is mutex unlocker. Used for unlocking mutex when execution goes out of
            the scope, event if we leaving the scope due to thrown exception.

            Usage:
            @code
                {
                    mutex_t::locker_t locker( mutex );
                    ...some code...
                }
            @endcode

        */
        class locker_t {

            public :

                locker_t( mutex_t & mutex );
                ~locker_t();

            private :

                locker_t( locker_t const & );

            private :

                mutex_t & _mutex;

        }; // class locker_t

    public :

        static int const default_type =
            #if NDEBUG
                PTHREAD_MUTEX_DEFAULT
            #else
                PTHREAD_MUTEX_ERRORCHECK
            #endif
            ;

        mutex_t( int type = default_type );
        ~mutex_t();

        void lock();
        void unlock();

    private :

        class attr_t {
            friend class mutex_t;
            public :
                attr_t( int type );
                ~attr_t();
            private :
                pthread_mutexattr_t  _attr;
        }; // class attr_t

    private :

        mutex_t( mutex_t const & );

    private :

        pthread_mutex_t     _mutex;

}; // class mutex_t


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#endif // _MUTEX_HPP_

// end of file //
