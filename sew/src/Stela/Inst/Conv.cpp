#include "../Codegen/Codegen_C.h"
#include "Conv.h"
#include "Type.h"

/**
*/
struct Conv : Inst {
    Conv( Type *dst ) : dst( dst ) {}
    virtual void write_dot( Stream &os ) { os << "Conv[" << *dst << "]"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Conv( dst ); }
    virtual Type *type() { return dst; }
    virtual void write( Codegen_C *cc, int prec ) {
        cc->on.write_beg();
        out_reg->write( cc, new_reg ) << " = ";
        bool p = out_reg->type != dst;
        if ( p ) {
            cc->write( dst );
            *cc->os << "(";
        }
        cc->write_out( inp[ 0 ] );
        if ( p )
            *cc->os << ")";
        cc->on.write_end( ";" );
    }
    Type *dst;
};

Expr conv( Type *dst, Expr inp ) {
    if ( dst == inp->type() )
        return inp;

    Conv *res = new Conv( dst );
    res->add_inp( inp );
    return res;
}

