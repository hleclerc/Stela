#include "../CodeGen/InstInfo_C.h"
#include "Type.h"
#include "Conv.h"

/**
*/
class Conv : public Inst {
public:
    Conv( Type *tr, Type *ta ) : tr( tr ), ta( ta ) {
    }
    virtual void write_dot( Stream &os ) const {
        os << "conv[" << *tr << "," << *ta << "]";
    }
    virtual int size() const {
        return tr->size();
    }
    virtual Ptr<Inst> forced_clone( Vec<Ptr<Inst> > &created ) const {
        return new Conv( tr, ta );
    }

    virtual Ptr<Inst> snapshot() {
        return conv( tr, ta, inp[ 0 ]->snapshot() );
    }
    virtual void write_1l_to( CodeGen_C *cc ) const  {
        *cc->os << *tr << "( " << cc->code( inp[ 0 ] ) << " )";
    }
    virtual Type *out_type_proposition( CodeGen_C *cc ) const {
        return tr;
    }
    virtual Type *inp_type_proposition( CodeGen_C *cc, int ninp ) const {
        return ta;
    }

    Type *tr;
    Type *ta;
};

Ptr<Inst> conv( Type *tr, Type *ta, const Ptr<Inst> &a ) {
    Conv *res = new Conv( tr, ta );
    res->add_inp( a );
    return res;
}
