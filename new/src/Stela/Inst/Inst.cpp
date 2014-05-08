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
    IP_ERROR( "not an Inst based on a Room instance" );
}

void Inst::set( Ptr<Inst> val ) {
    IP_ERROR( "not an Inst based on a Room instance" );
}

void Inst::add_inp( Ptr<Inst> val ) {
    val->par << Parent{ this, int( inp.size() ) };
    inp << val;
}

Ptr<Inst> simplified( Ptr<Inst> val ) {
    return val;
}
