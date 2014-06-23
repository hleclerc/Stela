#include "While.h"
#include "Ip.h"

/**
*/
struct WhileInp : Inst {
    WhileInp( const Vec<Type *> types ) : types( types ) {}
    virtual void write_dot( Stream &os ) { os << "while_inp"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new WhileInp( types ); }
    virtual Type *type( int nout ) { return types[ nout ]; }
    virtual Type *ptype( int nout ) { TODO; return 0; }
    virtual Type *type() { return ip->type_Void; }
    Vec<Type *> types;
};

/**
*/
struct WhileOut : Inst {
    WhileOut( const Vec<Vec<bool> > &pos ) : pos( pos ) {}
    virtual void write_dot( Stream &os ) { os << "while_out"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new WhileOut( pos ); }
    virtual Type *type( int nout ) { return inp[ nout ]->type(); }
    virtual Type *type() { return ip->type_Void; }
    Vec<Vec<bool> > pos;
};

/**
*/
struct WhileInst : Inst {
    WhileInst( const Vec<int> &corr ) : corr( corr ) {}
    virtual void write_dot( Stream &os ) { os << "while"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new WhileInst( corr ); }
    virtual Type *type( int nout ) { return ext[ 0 ]->type( nout ); }
    virtual Type *ptype( int nout ) { TODO; return 0; }
    virtual Type *type() { return ip->type_Void; }
    virtual int ext_disp_size() const { return 1; }
    Vec<int> corr;
};

Expr while_inp( const Vec<Type *> types ) {
    WhileInp *res = new WhileInp( types );
    return res;
}

Expr while_out( const Vec<Expr> &inp, const Vec<Vec<bool> > &pos ) {
    WhileOut *res = new WhileOut( pos );
    for( Expr i : inp )
        res->add_inp( i );
    return res;
}

Expr while_inst( const Vec<Expr> &inp, Expr winp, Expr wout, const Vec<int> &corr ) {
    WhileInst *res = new WhileInst( corr );
    for( Expr i : inp )
        res->add_inp( i );
    res->add_ext( wout );
    res->add_ext( winp );
    return res;
}
