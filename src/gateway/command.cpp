// command.cpp //


#include "command.hpp"
#include "error.hpp"


#include <cassert>
#include <climits>
#include <cstring>
#include <sstream>

#include <arpa/inet.h>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


static size_t const   align = sizeof( int );


// -------------------------------------------------------------------------------------------------
// command_t
// -------------------------------------------------------------------------------------------------

mark_t const command_t::wrp( "WRP" );
mark_t const command_t::wre( "WRE" );
mark_t const command_t::rdp( "RDP" );
mark_t const command_t::rde( "RDE" );
mark_t const command_t::rds( "RDS" );
mark_t const command_t::eoc( "$/\x80\x80" );
mark_t const command_t::ack( "S" );
mark_t const command_t::nak( "E" );


command_t *
command_t::bin_read(
    std::istream &      stream
) {

    static logger_t _logger( "Command" );

    mark_t type;
    type.read( stream );
    command_t * command = NULL;
    if ( 0 ) {
    } else if ( type == wrp ) {
        command = new wrp_command_t();
    } else if ( type == wre ) {
        command = new wre_command_t();
    } else if ( type == rdp ) {
        command = new rdp_command_t();
    } else if ( type == rde ) {
        command = new rdp_command_t();
    } else if ( type == rds ) {
        command = new rds_command_t();
    } else {
        ERR( 1, "Unknow packet type (" << c_literal( type.str() ) << ") read" );
    }; // if
    command->_bin_read( stream );
    mark_t end;
    end.read( stream );
    ERR(
        end != eoc,
        "Command terminator (" << c_literal( eoc.str() ) << ") expected "
            << "but read " << c_literal( end.str() )
    );
    return command;
}; // read


command_t::command_t(
    mark_t type
) :
    _type( type )
{
};


command_t::~command_t(
) {
};


mark_t
command_t::type(
) const {
    return _type;
};


void
command_t::bin_write(
    std::ostream &  stream
) const {
    _type.write( stream );
    _bin_write( stream );
    eoc.write( stream );
};


void
command_t::txt_write(
    std::ostream &  stream
) const {
    stream << _type << " ";
    _txt_write( stream );
};


// -------------------------------------------------------------------------------------------------
// wrp_command_t
// -------------------------------------------------------------------------------------------------


wrp_command_t::wrp_command_t(
) :
    command_t( wrp )
{
};


bool
wrp_command_t::operator ==(
    wrp_command_t const & rhs
) const {
    return type() == rhs.type() && samples == rhs.samples;
};


void
wrp_command_t::_bin_write(
    std::ostream &  stream
) const {
    write_packet( stream, samples );
};


void
wrp_command_t::_txt_write(
    std::ostream &  stream
) const {
    stream << samples;
};


void
wrp_command_t::_bin_read(
    std::istream & stream
) {
    read_packet( stream, samples );
};


// -------------------------------------------------------------------------------------------------
// wre_command_t
// -------------------------------------------------------------------------------------------------


wre_command_t::wre_command_t(
) :
    command_t( wre )
{
};


bool
wre_command_t::operator ==(
    wre_command_t const & rhs
) const {
    return type() == rhs.type() && events == rhs.events;
};


void
wre_command_t::_bin_write(
    std::ostream &  stream
) const {
    write_packet( stream, events );
};


void
wre_command_t::_txt_write(
    std::ostream &  stream
) const {
    stream << events;
};


void
wre_command_t::_bin_read(
    std::istream &  stream
) {
    read_packet( stream, events );
};


// -------------------------------------------------------------------------------------------------
// _read_command_t
// -------------------------------------------------------------------------------------------------


_read_command_t::_read_command_t(
    mark_t mark
) :
    command_t( mark )
{
};


void
_read_command_t::_bin_write(
    std::ostream &  stream
) const {
    write_packet( stream, channels, false );
};


void
_read_command_t::_txt_write(
    std::ostream &  stream
) const {
    stream << channels;
};


void
_read_command_t::_bin_read(
    std::istream & stream
) {
    read_packet( stream, channels, false );
};


// -------------------------------------------------------------------------------------------------
// rdp_command_t
// -------------------------------------------------------------------------------------------------


rdp_command_t::rdp_command_t(
) :
    _read_command_t( rdp )
{
};


// -------------------------------------------------------------------------------------------------
// rdp_command_t
// -------------------------------------------------------------------------------------------------


rde_command_t::rde_command_t(
) :
    _read_command_t( rde )
{
};


// -------------------------------------------------------------------------------------------------
// rds_command_t
// -------------------------------------------------------------------------------------------------


rds_command_t::rds_command_t(
) :
    command_t( rds )
{
};


bool
rds_command_t::operator ==(
    rds_command_t const & rhs
) const {
    return type() == rhs.type() && names == rhs.names;
};


void
rds_command_t::_bin_write(
    std::ostream & stream
) const {
    write_packet( stream, names );
};


void
rds_command_t::_txt_write(
    std::ostream &  stream
) const {
    stream << names;
};


void
rds_command_t::_bin_read(
    std::istream & stream
) {
    read_packet( stream, names );
};


std::ostream &
operator <<(
    std::ostream &      stream,
    command_t const &   command
) {
    command.txt_write( stream );
    return stream;
};

}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


// end of file //
