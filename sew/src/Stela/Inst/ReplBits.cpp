#include "../Codegen/Codegen_C.h"
#include "ReplBits.h"
#include "Slice.h"
#include "Type.h"
#include "Ip.h"
#include "Op.h"

/**
  repl_bits( src, off, val )
*/
struct ReplBits : Inst {
    ReplBits() {
        use_bytes_for_codegen = false;
    }
    virtual void write_dot( Stream &os ) const {
        os << "ReplBits";
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new ReplBits;
    }
    virtual Type *type() {
        return inp[ 0 ]->type();
    }
    virtual Expr _simp_slice( Type *dst, Expr off ) {
        SI32 beg, len = inp[ 2 ]->size();
        if ( not inp[ 1 ]->get_val( ip->type_SI32, &beg ) ) return (Inst *)0;
        SI32 nbeg, nlen = dst->size();
        if ( not off->get_val( ip->type_SI32, &nbeg ) ) return (Inst *)0;
        SI32 end = beg + len, nend = nbeg + nlen;
        //
        if ( nend <= beg or nbeg >= end ) return slice( dst, inp[ 0 ], off );
        if ( nbeg >= beg and nend <= end ) return slice( dst, inp[ 2 ], sub( off, inp[ 1 ] ) );

        return Inst::_simp_slice( dst, off );
    }
    virtual void get_constraints() {
        // inp[ 0 ] -> out
        add_same_out( 0, this, -1, COMPULSORY );
    }
    virtual void codegen_simplification( Vec<Expr> &created, Vec<Expr> &out ) {
        int m;
        if ( use_bytes_for_codegen == false and inp[ 1 ]->op_num() == ID_OP_mul and inp[ 1 ]->inp[ 0 ]->get_val( ip->type_SI32, &m ) and m == 8 ) {
            mod_inp( inp[ 1 ]->inp[ 1 ], 1 );
            use_bytes_for_codegen = true;
        }
    }
    virtual void write( Codegen_C *cc, CC_SeqItemBlock **b ) {
        cc->on.write_beg() << "*(" << cc->type_to_str( inp[ 2 ]->out_reg->type ) << " *)( (char *)&";
        out_reg->write( cc, false );
        *cc->os << " + ";
        inp[ 1 ]->out_reg->write( cc, false );
        if ( not use_bytes_for_codegen )
            *cc->os << " / 8";
        *cc->os << " ) = ";
        inp[ 2 ]->out_reg->write( cc, new_reg );
        cc->on.write_end( ";" );
    }
    bool use_bytes_for_codegen;
};

Expr repl_bits( Expr src, Expr off, Expr val ) {
    SI32 voff;
    // replace all the bits (but keep the same type)
    if ( off->get_val( ip->type_SI32, &voff ) and voff == 0 and src->size() == val->size() )
        return rcast( src->type(), val );
    //
    if ( val->size() == 0 )
        return src;
    // possible simplification
    if ( Expr res = src->_simp_repl_bits( off, val ) )
        return res;

    ReplBits *res = new ReplBits;
    res->add_inp( src );
    res->add_inp( off );
    res->add_inp( val );
    return res;
}

