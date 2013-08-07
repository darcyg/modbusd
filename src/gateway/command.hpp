// command.hpp //

#ifndef _COMMAND_HPP_
#define _COMMAND_HPP_


#include "data.hpp"
#include "string.hpp"

#include <vector>


namespace stek {
namespace oasis {
namespace ic {
namespace dbgateway {


class command_t {

    public :

        static mark_t const wrp;
        static mark_t const wre;
        static mark_t const rdp;
        static mark_t const rde;
        static mark_t const rds;
        static mark_t const eoc;
        static mark_t const ack;
        static mark_t const nak;

        static command_t * bin_read( std::istream & stream );

    public :

        command_t( mark_t type );
        virtual ~command_t();

        mark_t   type() const;
        void     bin_write( std::ostream & stream ) const;
        void     txt_write( std::ostream & stream ) const;

    protected :

        virtual void   _bin_write( std::ostream & stream ) const = 0;
        virtual void   _txt_write( std::ostream & stream ) const = 0;
        virtual void   _bin_read( std::istream & stream ) = 0;

    private :

        mark_t const _type;

}; // class command_t


class wrp_command_t : public command_t {

    public :

        wrp_command_t();
        bool operator ==( wrp_command_t const & rhs ) const;

    protected :

        virtual void   _bin_write( std::ostream & stream ) const;
        virtual void   _txt_write( std::ostream & stream ) const;
        virtual void   _bin_read( std::istream & stream );

    public :

        samples_t   samples;

}; // class wrp_command_t


class wre_command_t : public command_t {

    public :

        wre_command_t();
        bool operator ==( wre_command_t const & rhs ) const;

    protected :

        virtual void   _bin_write( std::ostream & stream ) const;
        virtual void   _txt_write( std::ostream & stream ) const;
        virtual void   _bin_read( std::istream & stream );

    public :

        _events_t    events;

}; // struct wre_command_t


class _read_command_t : public command_t {

    protected :

        _read_command_t( mark_t mark );

        virtual void   _bin_write( std::ostream & stream ) const;
        virtual void   _txt_write( std::ostream & stream ) const;
        virtual void   _bin_read( std::istream & stream );

    public :

        channels_t   channels;

}; // class _read_command_t


class rdp_command_t : public _read_command_t {

    public :

        rdp_command_t();

}; // class rdp_command_t


class rde_command_t : public _read_command_t {

    public :

        rde_command_t();

}; // class rde_command_t


class rds_command_t : public command_t {

    public :

        rds_command_t();
        bool operator ==( rds_command_t const & rhs ) const;

    protected :

        virtual void   _bin_write( std::ostream & stream ) const;
        virtual void   _txt_write( std::ostream & stream ) const;
        virtual void   _bin_read( std::istream & stream );

    public :

        vstrings_t names;

}; // class rds_command_t


std::ostream &
operator <<(
    std::ostream &      stream,
    command_t const &   command
);


}; // namespace dbgateway
}; // namespace ic
}; // namespace oasis
}; // namespace stek


#endif // _COMMAND_HPP_

// end of file //
