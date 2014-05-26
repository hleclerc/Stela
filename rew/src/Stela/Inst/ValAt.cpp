#include "CppOperatorPrecedence.h"
#include "InstInfo_C.h"
#include "ValAt.h"

/**
*/
class ValAt : public Inst {
public:
    ValAt( int len ) : len( len ) {}
    virtual void write_dot( Stream &os ) const { os << "at"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new ValAt( len ); }
    virtual int size() const { return len; }
    virtual void write_to( Codegen_C *cc, int prec ) {
        if ( prec >= 0 ) {
            if ( inp[ 0 ]->is_a_Room() )
                *cc->os << *IIC( inp[ 0 ] )->out_reg;
            else
                *cc->os << "*" << cc->code( inp[ 0 ], CppOperatorPrecedence::POINTED_DATA );
        }
    }

    int len;
};

Expr val_at( Expr ptr, int len ) {
    ValAt *res = new ValAt( len );
    res->add_inp( ptr );
    return res;
}
