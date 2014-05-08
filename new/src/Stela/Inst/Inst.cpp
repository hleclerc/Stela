#include "Inst.h"
#include "Ip.h"

Inst::~Inst() {
}

void Inst::write_to_stream( Stream &os ) const {
    write_dot( os );
    if ( inp.size() ) {
        for( int i = 0; i < inp.size(); ++i )
            os << ( i ? "," : "(" ) << inp[ i ];
        os << ")";
    }
}

void Inst::add_var_ptr( Var *var ) {
    write_dot( std::cerr );
    IP_ERROR( "not an Inst based on a Room instance" );
}

void Inst::set( Ptr<Inst> val ) {
    write_dot( std::cerr );
    IP_ERROR( "not an Inst based on a Room instance" );
}

void Inst::add_inp( Ptr<Inst> val ) {
    val->par << Parent{ this, int( inp.size() ) };
    inp << val;
}

Ptr<Inst> Inst::_simplified() {
    return 0;
}

Ptr<Inst> Inst::_pointer_on( int beg, int len ) {
    return 0;
}

Ptr<Inst> simplified( Ptr<Inst> val ) {
    if ( Ptr<Inst> res = val->_simplified() )
        return res;
    return val;
}
