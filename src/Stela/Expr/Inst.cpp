#include "Expr.h"

namespace Expr_NS {

PI64 Inst::cur_op_id = 0;

Inst::Inst() {
    ext_parent = 0;

    op_id_viz  = 0;
    op_id      = 0;
    op_mp      = 0;
}

Inst::~Inst() {
}

int Inst::size_in_bytes( int nout ) const {
    return ( size_in_bits( nout ) + 7 ) / 8;
}

const PI8 *Inst::cst_data( int nout ) const {
    return 0;
}

bool Inst::equal( const Inst *b ) const {
    if ( inst_id() != b->inst_id() or inp_size() != b->inp_size() )
        return false;
    for( int i = 0; i < inp_size(); ++i )
        if ( inp_expr( i ) != b->inp_expr( i ) )
            return false;
    return true;
}

Inst *Inst::factorized( Inst *inst ) {
    if ( inst->inp_size() ) {
        const Vec<Inst::Out::Item,-1,1> &p = inst->inp_expr( 0 ).parents();
        for( int i = 0; i < p.size(); ++i ) {
            if ( p[ i ].inst != inst and p[ i ].inst->equal( inst ) ) {
                delete inst;
                return p[ i ].inst;
            }
        }
    }
    return inst;
}

}
