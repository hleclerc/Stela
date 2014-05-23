#include "../System/AssignIfNeq.h"
#include "InstInfo_C.h"
#include "BoolOpSeq.h"
#include "SelectDep.h"
#include "Op.h"
#include "Ip.h"

/**
  inp[ 0 ] ? dep[ 0 ] : dep[ 1 ]
*/
class SelectDep : public Inst {
public:
    virtual void write_dot( Stream &os ) const {
        os << "?";
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new SelectDep;
    }
    virtual int size() const {
        return 0;
    }
    virtual Expr _simplified() {
        return 0;
    }
    virtual Expr _get_val() {
         return this;
    }
    virtual void update_when( const BoolOpSeq &cond ) {
        if ( not when )
            when = new BoolOpSeq( cond );
        else if ( not assign_if_neq( *when, *when or cond ) )
            return;

        BoolOpSeq binp = inp[ 0 ]->get_BoolOpSeq();

        inp[ 0 ]->update_when( cond );

        dep[ 0 ]->update_when( cond and binp );
        dep[ 1 ]->update_when( cond and not binp );
        for( int i = 2; i < dep.size(); ++i )
            dep[ i ]->update_when( cond );
    }
    virtual void update_out_type() {
        inp[ 0 ]->out_type_proposition( &ip->type_Bool );
    }
};

Expr select_dep( Expr cond, Expr ok_dep, Expr ko_dep ) {
    if ( int res = cond->always_checked() )
        return res > 0 ? ok_dep : ko_dep;

    SelectDep *res = new SelectDep;
    res->add_inp( cond );
    res->add_dep( ok_dep );
    res->add_dep( ko_dep );
    return res;
}
