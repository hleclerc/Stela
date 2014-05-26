#include "InstInfo_C.h"
#include "Syscall.h"
#include "Ip.h"

/**
*/
class Syscall : public Inst {
public:
    virtual void write_dot( Stream &os ) const {
        os << "syscall";
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new Syscall;
    }
    virtual int size() const {
        return ip->type_ST->size();
    }
    virtual Expr _get_val( int len ) {
        return this;
    }
    virtual void write_to( Codegen_C *cc, int prec ) {
        cc->on.write_beg() <<  "syscall( " ;
        for( int i = 0; i < inp.size(); ++i ) {
            if ( i )
                *cc->os << ", ";
            *cc->os << cc->code( inp[ i ] );
        }
        cc->on.write_end( " );" );
    }
    virtual void update_out_type() {
        out_type_proposition( ip->type_ST );
        for( int i = 0; i < inp.size(); ++i )
            inp[ i ]->out_type_proposition( ip->type_ST );
    }
};

Expr syscall( const Vec<Expr> &inp, Expr state ) {
    Syscall *res = new Syscall;
    for( Expr i : inp )
        res->add_inp( i );
    res->add_dep( state );
    return res;
}
