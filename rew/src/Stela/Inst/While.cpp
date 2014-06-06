#include "While.h"


/**
*/
class While : public Inst {
public:
    While( const Vec<int> &corr ) : corr( corr ) {}
    virtual void write_dot( Stream &os ) const { os << "while"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new While( corr ); }
    virtual int size() const { return 0; }
    virtual int size_out( int nout ) const { return ext[ 0 ]->inp[ nout ]->size(); }
    virtual int ext_disp_size() const { return 1; }

    Vec<int> corr;
};

/**
*/
class WhileInp : public Inst {
public:
    WhileInp( const Vec<int> &sizes ) : sizes( sizes ) {}
    virtual void write_dot( Stream &os ) const { os << "while_inp"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new WhileInp( sizes ); }
    virtual int size() const { return 0; }
    virtual int size_out( int nout ) const { return sizes[ nout ]; }
    Vec<int> sizes;
};
/**
*/
class WhileOut : public Inst {
public:
    virtual void write_dot( Stream &os ) const { os << "while_out"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new WhileOut; }
    virtual int size() const { return 0; }
};

Expr while_inst( const Vec<Expr> &inp_exprs, Expr winp, Expr wout, const Vec<int> &corr ) {
    While *res = new While( corr );
    for( Expr e : inp_exprs )
        res->add_inp( e );
    res->add_ext( wout );
    res->add_ext( winp );
    return res;
}

Expr while_inp( const Vec<int> &inp_sizes ) {
    return new WhileInp( inp_sizes );
}

Expr while_out( const Vec<Expr> &out_exprs ) {
    WhileOut *res = new WhileOut;
    for( Expr e : out_exprs )
        res->add_inp( e );
    return res;
}
