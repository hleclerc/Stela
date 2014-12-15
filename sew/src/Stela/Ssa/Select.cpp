#include "Select.h"

struct Select : Inst {
    virtual void write_dot( Stream &os ) const { os << "Select"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Select( pos ); }
    // virtual Type *type() { return inp[ 0 ]->type(); }
    virtual Expr simplified( Expr cond ) {
        //        BoolOpSeq bos = get_bos();
        //        if ( cond.imply( bos ) )
        //            return inp[ 0 ];
        //        if ( cond.imply( not bos ) )
        //            return inp[ 1 ];
        return this;
    }
    virtual void set( Expr obj, Expr cond ) {
        TODO;
    }
    virtual Expr get( Expr cond ) {
        // return select( get_bos() - cond, inp[ 0 ]->get( cond ), inp[ 1 ]->get( cond ) );
        TODO;
        return Expr();
    }
};

Expr select( Expr cond, Expr ok, Expr ko ) {
    if ( ok == ko ) return ok;
    if ( cond->always( true  ) ) return ok;
    if ( cond->always( false ) ) return ko;

    Select *res = new Select;
    res->add_inp( cond );
    res->add_inp( ok );
    res->add_inp( ko );
    return Inst::val_or_twin( res );
}
