// cache.tpp //

/**
    @file
    @brief Cache template implementation.
*/

#ifndef _CACHE_TPP_
#define _CACHE_TPP_


#include "cache.hpp"


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


template<
    typename item_t
>
_cache_t< item_t >::_cache_t(
    string_t const &    name
) :
    object_t( name + " cache" )
{
}; // ctor


template<
    typename item_t
>
string_t
_cache_t< item_t >::name(
) const {
    return _name;
};


template<
    typename item_t
>
typename _cache_t< item_t >::items_t
_cache_t< item_t >::get(
    keys_t const & keys
) const {
    mutex_t::locker_t locker( _mutex );
    items_t items;
    for ( size_t i = 0; i < keys.size(); ++ i ) {
        const_iterator_t it = _bulk.find( keys[ i ] );
        if ( it != _bulk.end() ) {
            items.push_back( it->second );
        };
    };
    return items;
}; // get


template<
    typename item_t
>
void
_cache_t< item_t >::update(
    items_t const & items
) {
    if ( items.empty() ) {
        DLOG( "nothing to do" );
    } else {
        DLOG( items.size() << " item(s) received" );
        size_t updated = 0;
        {
            mutex_t::locker_t locker( _mutex );
            for ( size_t i = 0; i < items.size(); ++ i ) {
                item_t const & item = items[ i ];
                iterator_t it = _bulk.find( item.key() );
                if ( it == _bulk.end() || item.date > it->second.date ) {
                    _bulk[ item.key() ] = item;
                    ++ updated;
                }; // if
            }; // for i
        }
        DLOG( updated << " item(s) updated" );
    }; // if
}; // update


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#endif // _CACHE_TPP_

// end of file //
