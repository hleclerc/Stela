#include "BoolOpSeq.h"
#include "Slice.h"
#include "Type.h"
#include "Op.h"
#include "Ip.h"

/**
   slice[ out_type ]( var, offset )
*/
struct Slice : Inst {
    Slice( Type *out_type ) : out_type( out_type ) {}
    virtual void write_dot( Stream &os ) { os << "Slice"; }
    virtual void write_to_stream( Stream &os, int prec ) {
        int voff;
        if ( inp[ 1 ]->get_val( ip->type_SI32, &voff ) and voff == 0 )
            os << "rcast[" << *out_type << "]( " << inp[ 0 ] << ")";
        else
            Inst::write_to_stream( os, prec );
    }

    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Slice( out_type ); }
    virtual Type *type() { return out_type; }

    virtual Expr _simp_slice( Type *dst, Expr off ) {
        if ( Expr res = Inst::_simp_slice( dst, off ) )
            return res;
        return inp[ 0 ]->_simp_slice( dst, add( off, inp[ 1 ] ) );
    }
    virtual Expr get( const BoolOpSeq &cond ) {
        int voff;
        if ( inp[ 1 ]->get_val( ip->type_SI32, &voff ) and voff == 0 ) {
            if ( out_type->orig != ip->class_Ptr ) {
                PRINT( *out_type );
                ERROR( "" );
                return ip->ret_error( "expecting a ptr" );
            }
            Type *val_type = ip->type_from_type_var( out_type->parameters[ 0 ] );
            return rcast( val_type, inp[ 0 ]->get( cond ) );
        }
        TODO;
        return 0;
    }
    virtual void set( Expr obj, const BoolOpSeq &cond ) {
        int voff;
        if ( inp[ 1 ]->get_val( ip->type_SI32, &voff ) and voff == 0 )
            return inp[ 0 ]->set( obj, cond );
        TODO;
    }

    Type *out_type;
};

Expr slice( Type *dst, Expr var, Expr off ) {
    if ( Expr res = var->_simp_slice( dst, off ) )
        return res;

    Slice *res = new Slice( dst );
    res->add_inp( var );
    res->add_inp( off );
    return res;
}

Expr rcast( Type *dst, Expr var ) {
    ASSERT( dst != ip->type_SI64, "..." );
    return slice( dst, var, 0 );
}
