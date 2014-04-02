#include "InstVisitor.h"
#include "While.h"
#include "Inst_.h"

class WhileInst : public Inst_<-1,-1,2> {
public:
    virtual int size_in_bits( int nout ) const { return ext[ 0 ]->size_in_bits( nout ); }
    virtual void write_dot( Stream &os ) const { os << "while_inst"; }
    virtual void apply( InstVisitor &visitor ) const { visitor.while_inst( *this ); }
    virtual int inst_id() const { return Inst::Id_WhileInst; }
    virtual int ext_size_disp() const { return 1; }

    Vec<int> corr_inp;
};

class WhileInp : public Inst_<-1,0> {
public:
    virtual int size_in_bits( int nout ) const { return _sizes_in_bits[ nout ]; }
    virtual void write_dot( Stream &os ) const { os << "while_inp"; }
    virtual void apply( InstVisitor &visitor ) const { visitor.while_inp( *this ); }
    virtual int inst_id() const { return Inst::Id_WhileInp; }

    Vec<int> _sizes_in_bits;
};

class WhileOut : public Inst_<0,-1> {
public:
    virtual int size_in_bits( int nout ) const { return inp[ nout ].size_in_bits(); }
    virtual void write_dot( Stream &os ) const { os << "while_out"; }
    virtual void apply( InstVisitor &visitor ) const { visitor.while_out( *this ); }
    virtual int inst_id() const { return Inst::Id_WhileOut; }
};


Inst *while_out( const Vec<Expr> &out_exprs ) {
    WhileOut *res = new WhileOut;
    for( int i = 0; i < out_exprs.size(); ++i )
        res->inp_push( out_exprs[ i ] );
    return res;
}

Inst *while_inp( const Vec<int> &sizes_in_bits ) {
    WhileInp *res = new WhileInp;
    res->out_set_size( sizes_in_bits.size() );
    res->_sizes_in_bits = sizes_in_bits;
    return res;
}

Inst *while_inst( const Vec<Expr> &inp_exprs, const Inst *winp, const Inst *wout, const Vec<int> &corr_inp ) {
    WhileInst *res = new WhileInst;
    res->corr_inp = corr_inp;
    res->out_set_size( corr_inp.size() );
    for( int i = 0; i < inp_exprs.size(); ++i )
        res->inp_push( inp_exprs[ i ] );
    res->ext_repl( 0, wout );
    res->ext_repl( 1, winp );
    return res;
}
