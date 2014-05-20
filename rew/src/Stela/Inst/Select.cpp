#include "InstInfo_C.h"
#include "Select.h"
#include "Op.h"
#include "Ip.h"

/**
  inp[ 0 ] ? inp[ 1 ] : inp[ 2 ]
*/
class Select : public Inst {
public:
    virtual void write_dot( Stream &os ) const {
        os << "select";
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new Select;
    }
    virtual int size() const {
        return inp[ 1 ]->size();
    }
    virtual Expr _simplified() {
        if ( int res = inp[ 0 ]->checked_if( ip->cur_cond() ) )
            return res > 0 ? inp[ 1 ] : inp[ 2 ];
        return 0;
    }
    virtual void update_when( Expr cond ) {
        if ( when ) {
            Expr res = op( &ip->type_Bool, &ip->type_Bool, when, &ip->type_Bool, cond, Op_or_boolean() );
            if ( when == res )
                return;
            res->update_when( cond );
            when = res;
        } else
            when = cond;

        Expr cok = inp[ 0 ];
        Expr cko = op( &ip->type_Bool, &ip->type_Bool, cok, Op_not_boolean() );
        Expr c0 = op( &ip->type_Bool, &ip->type_Bool, cok, &ip->type_Bool, cond, Op_and_boolean() );
        Expr c1 = op( &ip->type_Bool, &ip->type_Bool, cko, &ip->type_Bool, cond, Op_and_boolean() );
        c0->update_when( cond );
        c1->update_when( cond );
        inp[ 0 ]->update_when( cond );
        inp[ 1 ]->update_when( c0 );
        inp[ 2 ]->update_when( c1 );

        for( Expr inst : dep )
            inst->update_when( cond );
    }
    virtual bool is_a_Select() const {
        return true;
    }
};

Expr select( Expr cond, Expr ok, Expr ko ) {
    if ( int res = cond->always_checked() )
        return res > 0 ? ok : ko;

    Select *res = new Select;
    res->add_inp( cond );
    res->add_inp( ok );
    res->add_inp( ko );
    return res;
}
