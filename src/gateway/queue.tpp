// queue.tpp //

/**
    @file
    @brief Queue template implementation.
*/

#ifndef _QUEUE_TPP_
#define _QUEUE_TPP_


#include "queue.hpp"

#include <cassert>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


// -------------------------------------------------------------------------------------------------
// queue_t class
// -------------------------------------------------------------------------------------------------


template<
    typename item_t
>
queue_t< item_t >::queue_t(
    string_t const & name
) :
    object_t( name + " queue" ),
    _mutex( PTHREAD_MUTEX_RECURSIVE )
{
}; // ctor


template<
    typename item_t
>
queue_t< item_t >::~queue_t(
) {
    if ( ! _bulk.empty() ) {
        ELOG( "Non-empty queue deleted" );
    }; // if
}; // dtor


/**
    @brief Add item to the queue.
*/
template<
    typename item_t
>
void
queue_t< item_t >::push_back(
    item_t const &  item
) {
    mutex_t::locker_t locker( _mutex );
    _bulk.push_back( item );
}; // push_back


/**
    @brief Add items to the queue.

    This method provides synchronized access to the queue. If one thread works with the queue, other
    will be blocked.

    After adding items to the queue, the method updates associated cache.
*/
template<
    typename item_t
>
void
queue_t< item_t >::push_back(
    items_t const &  items
) {
    if ( ! items.empty() ) {
        mutex_t::locker_t locker( _mutex );
        _bulk.reserve( _bulk.size() + items.size() );
        for ( size_t i = 0; i < items.size(); ++ i ) {
            _bulk.push_back( items[ i ] );
        };
    };
}; // push_back


/**
    @brief Grab all items to the queue.

    The method provides synchronized access to the queue. If one thread works with the queue,
    other will be blocked.
*/
template<
    typename item_t
>
typename queue_t< item_t >::items_t
queue_t< item_t >::grab(
) {
    items_t result;
    {
        mutex_t::locker_t locker( _mutex );
        _bulk.swap( result );
    }
    return result;
}; // grab


// -------------------------------------------------------------------------------------------------
// cached_queue_t class
// -------------------------------------------------------------------------------------------------

template<
    typename item_t
>
cached_queue_t< item_t >::cached_queue_t(
    string_t const & name,
    cache_t &        cache
) :
    queue_t< item_t >( name ),
    _cache( cache )
{
}; // ctor


/**
    @brief Add item to the queue.
*/
template<
    typename item_t
>
void
cached_queue_t< item_t >::push_back(
    item_t const &  item
) {
    mutex_t::locker_t locker( queue_t< item_t >::_mutex );
    queue_t< item_t >::push_back( item );
    _cache.update( item );
}; // push_back


/**
    @brief Add items to the queue.

    This method provides synchronized access to the queue. If one thread works with the queue,
    other will be blocked.

    After adding items to the queue, the method updates associated cache.
*/
template<
    typename item_t
>
void
cached_queue_t< item_t >::push_back(
    items_t const &  items
) {
    mutex_t::locker_t locker( queue_t< item_t >::_mutex );
    queue_t< item_t >::push_back( items );
    _cache.update( items );
}; // push_back


/**
    @brief Return reference to the associated cache.
*/
template<
    typename item_t
>
typename cached_queue_t< item_t >::cache_t &
cached_queue_t< item_t >::cache(
) {
    return _cache;
}; // cache


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#endif // _QUEUE_TPP_

// end of file //
