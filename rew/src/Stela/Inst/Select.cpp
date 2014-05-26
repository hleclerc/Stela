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
    virtual void write_to( Codegen_C *cc, int prec ) {
    }
    virtual void inp_type_proposition( Type *type, int ninp ) {
        if ( ninp >= 1 and ninp <= 2 ) {
            inp[ 3 - ninp ]->out_type_proposition( type );
            out_type_proposition( type );
        }
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
