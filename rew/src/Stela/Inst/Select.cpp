#include "../System/AssignIfNeq.h"
#include "InstInfo_C.h"
#include "BoolOpSeq.h"
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
    virtual void update_when( const BoolOpSeq &cond ) {
        if ( not when )
            when = new BoolOpSeq( cond );
        else if ( not assign_if_neq( *when, *when or cond ) )
            return;

        BoolOpSeq binp = inp[ 0 ]->get_BoolOpSeq();

        inp[ 0 ]->update_when( cond );
        inp[ 1 ]->update_when( cond and binp );
        inp[ 2 ]->update_when( cond and not binp );

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
