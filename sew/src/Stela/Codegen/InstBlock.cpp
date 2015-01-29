#include "InstBlock.h"
#include "Codegen.h"

InstBlock::InstBlock( InstBlock *parent ) : parent( parent ) {
    use_out_reg = 0;
}

Expr InstBlock::forced_clone( Vec<Expr> &created ) const {
    TODO;
    return this;
}

void InstBlock::write_dot( Stream &os ) const {
    os << "Block";
}

bool InstBlock::need_out_reg() {
    return false;
}

void InstBlock::operator<<( Inst *inst ) {
    inst->parent_block = this;
    seq << inst;
}

int InstBlock::add_use( OutReg *reg ) {
    if ( use_out_reg == reg )
        return ++use_cpt;
    use_out_reg = reg;
    return ( use_cpt = 1 );
}

void InstBlock::add_decl( OutReg *out_reg ) {
    decl[ out_reg->type ] << out_reg;
}

void InstBlock::write( Codegen *c ) {
    for( auto &m : decl )
        c->write_decl( m.first, m.second );
    for( Expr e : seq )
        e->write( c );
}
