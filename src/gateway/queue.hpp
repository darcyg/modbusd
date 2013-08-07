// queue.hpp //

/**
    @file
    @brief Queue interface.
*/

#ifndef _QUEUE_HPP_
#define _QUEUE_HPP_


#include "cache.hpp"
#include "data.hpp"
#include "mutex.hpp"
#include "string.hpp"

#include <vector>
#include <map>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


/**
    @brief Queue is a synchronized collection of items.

    Queue is synchronous: several threads may simultaneously add or grab items to/from the queue.
*/
template<
    typename item_t
>
class queue_t : public object_t {

    public :

        typedef std::vector< item_t > items_t;

        queue_t( string_t const & name );
        ~queue_t();

        void        push_back( item_t const & items );
        void        push_back( items_t const & items );
        items_t     grab();

    private :

        queue_t( queue_t const & );     // Not copyable.

    protected :

        items_t     _bulk;      ///< Bulk of the queue: items not-yet-written to a database.
        mutex_t     _mutex;

}; // class queue_t


/**
    @brief Cached queue is a queue with associated cache.

    As soon as an item (or items) is added to the queue, the queue updates the associated cache.
*/
template<
    typename item_t
>
class cached_queue_t : public queue_t< item_t > {

    public :

        typedef typename queue_t< item_t >::items_t items_t;
        typedef _cache_t< item_t >                  cache_t;

        cached_queue_t( string_t const & name, cache_t & cache );

        void        push_back( item_t const & item );
        void        push_back( items_t const & items );
        cache_t &   cache();

    private :

        cached_queue_t( cached_queue_t const & );     // Not copyable.

    protected :

        cache_t &   _cache;     ///< Cache associated with the queue.

}; // class _queue_t


typedef cached_queue_t< sample_t >    sample_queue_t;
typedef cached_queue_t< event_t >     event_queue_t;
typedef cached_queue_t< setting_t >   setting_queue_t;


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#include "queue.tpp"


#endif // _QUEUE_HPP_

// end of file //
