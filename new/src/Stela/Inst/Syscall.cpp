#include "../CodeGen/InstInfo_C.h"
#include "../CodeGen/CodeGen_C.h"
#include "Syscall.h"
#include "Ip.h"

/**
*/
class Syscall : public Inst {
public:
    Syscall() {}
    virtual void write_dot( Stream &os ) const {
        os << "syscall";
    }
    virtual int size() const {
        return inp.size() ? inp[ 0 ]->size() : 0;
    }
    virtual Ptr<Inst> forced_clone( Vec<Ptr<Inst> > &created ) const {
        return new Syscall;
    }
    virtual void set( Ptr<Inst> val ) {
        dep[ 0 ]->set( val );
    }
    virtual void write_1l_to( CodeGen_C *cc ) const {
        *cc->os << "syscall( ";
        for( int i = 0; i < inp.size(); ++i ) {
            *cc->os << ( i ? ", " : "" ) << cc->code( inp[ i ] );
        }
        *cc->os << " )";
    }
    virtual Type *out_type_proposition( CodeGen_C *cc ) const {
        return &ip->type_SI64;
    }
    virtual Type *inp_type_proposition( CodeGen_C *cc, int ninp ) const {
        return &ip->type_SI64;
    }
};

Ptr<Inst> syscall( const Vec<Ptr<Inst> > children, const Vec<Ptr<Inst> > &cnd ) {
    Syscall *res = new Syscall;
    for( const Ptr<Inst> &ch : cnd )
        res->add_cnd( ch );
    for( const Ptr<Inst> &ch : children )
        res->add_inp( ch );
    return res;
}
