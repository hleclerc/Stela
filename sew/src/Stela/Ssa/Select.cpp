#include "Select.h"
#include "Op.h"

struct Select : Inst {
    virtual void write_dot( Stream &os ) const { os << "Select"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Select; }
    virtual Type *type() { return inp[ 1 ]->type(); }
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
        return select( inp[ 0 ], inp[ 1 ]->get( cond ), inp[ 2 ]->get( cond ) );
    }
};

Expr select( Expr cond, Expr ok, Expr ko ) {
    if ( ok == ko ) return ok;
    if ( cond->always( true  ) ) return ok;
    if ( cond->always( false ) ) return ko;

    if ( cond->op_type() == ID_OP_not_boolean )
        return select( cond->inp[ 0 ], ko, ok );

    Select *res = new Select;
    res->add_inp( cond );
    res->add_inp( ok );
    res->add_inp( ko );
    return Inst::twin_or_val( res );
}
