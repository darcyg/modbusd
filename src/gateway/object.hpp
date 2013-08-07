// object.hpp //

#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_


#include "string.hpp"
#include "logger.hpp"


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {

class object_t {
    public :
        object_t( string_t name ) : _name( name ), _logger( name ) {};
        object_t( object_t const & parent, string_t name ) : _parent( parent._name ), _name( name ), _logger( parent._name + ": " + name ) {};
        string_t name() const { return _name; };
        logger_t & logger() { return _logger; };
        void rename( string_t const & name ) { DLOG( "i am `" << name << "' now" ); _name = name; _logger = logger_t( _parent + ": " + name ); };
    private :
        string_t _parent;   ///< Name of parent object.
        string_t _name;     ///< Nane of object.
    protected :
        logger_t _logger;
}; // class object_t


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#endif // _OBJECT_HPP_

// end of file //
