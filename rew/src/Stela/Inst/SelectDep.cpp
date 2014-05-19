#include "SelectDep.h"
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
        if ( int res = inp[ 0 ]->checked_if( ip->cur_cond() ) )
            return res > 0 ? dep[ 0 ] : dep[ 1 ];
        return 0;
    }
    virtual Expr _get_val() {
         return this;
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
