// cache.hpp //

/**
    @file
    @brief Cache interface.
*/

#ifndef _CACHE_HPP_
#define _CACHE_HPP_


#include "data.hpp"
#include "string.hpp"
#include "mutex.hpp"
#include "object.hpp"

#include <vector>
#include <map>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


/**
*/
template<
    typename item_t
>
class _cache_t : public object_t {

    public :

        typedef std::vector< item_t >   items_t;
        typedef typename item_t::key_t  key_t;
        typedef std::vector< key_t >    keys_t;

        _cache_t( string_t const & name );

        string_t name()                     const;
        items_t  get( keys_t const & keys ) const;

        void update( items_t const & items );

    private :

        typedef std::map< typename item_t::key_t, item_t >  map_t;
        typedef typename map_t::const_iterator              const_iterator_t;
        typedef typename map_t::iterator                    iterator_t;

        map_t               _bulk;
        mutable mutex_t     _mutex;

}; // _cache_t


typedef _cache_t< sample_t >    sample_cache_t;
typedef _cache_t< event_t >     event_cache_t;
typedef _cache_t< setting_t >   setting_cache_t;


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#include "cache.tpp"


#endif // _CACHE_HPP_

// end of file //
