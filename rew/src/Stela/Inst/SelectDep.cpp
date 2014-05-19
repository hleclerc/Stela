#include "InstInfo_C.h"
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
        if ( int res = inp[ 0 ]->checked_if( ip->cur_cond() ) )
            return res > 0 ? dep[ 0 ] : dep[ 1 ];
        return 0;
    }
    virtual Expr _get_val() {
         return this;
    }
    virtual void _update_when_C( Expr cond ) {
        Expr res = op( &ip->type_Bool, &ip->type_Bool, IIC( this )->when, &ip->type_Bool, cond, Op_or_boolean() );
        if ( IIC( this )->when == res )
            return;

        IIC( this )->when = res;

        inp[ 0 ]->_update_when_C( cond );

        Expr nc = op( &ip->type_Bool, &ip->type_Bool, inp[ 0 ], Op_not_boolean() );
        Expr c0 = op( &ip->type_Bool, &ip->type_Bool, inp[ 0 ], &ip->type_Bool, cond, Op_and_boolean() );
        Expr c1 = op( &ip->type_Bool, &ip->type_Bool, nc      , &ip->type_Bool, cond, Op_and_boolean() );
        dep[ 0 ]->_update_when_C( c0 );
        dep[ 1 ]->_update_when_C( c1 );
        for( int i = 2; i < dep.size(); ++i )
            dep[ i ]->_update_when_C( cond );
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
