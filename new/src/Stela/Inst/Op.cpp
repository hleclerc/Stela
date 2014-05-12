#include "../CodeGen/InstInfo_C.h"
#include "Type.h"
#include "Op.h"

template<class T>
struct OpCout {};

template<> struct OpCout<Op_add> {
    enum { oper = 1 };
    void write_to_stream( Stream &os ) const { os << "+"; }
};

/**
*/
template<class T>
class Op : public Inst {
public:
    Op( Type *tr, Type *ta, Type *tb ) : tr( tr ), ta( ta ), tb( tb ) {
    }
    virtual void write_dot( Stream &os ) const {
        os << T();
    }
    virtual int size() const {
        return tr->size();
    }
    virtual Ptr<Inst> forced_clone( Vec<Ptr<Inst> > &created ) const {
        return new Op( tr, ta, tb );
    }
    virtual void write_1l_to( CodeGen_C *cc ) const {
        if ( OpCout<T>::oper )
            *cc->os
                << cc->code( inp[ 0 ] ) << ' '
                << OpCout<T>() << ' '
                << cc->code( inp[ 1 ] );
        else
            *cc->os
                << OpCout<T>() << "( "
                << cc->code( inp[ 0 ] ) << ", "
                << cc->code( inp[ 1 ] ) << " )";
    }
    virtual Type *out_type_proposition( CodeGen_C *cc ) const {
        return tr;
    }
    virtual Type *inp_type_proposition( CodeGen_C *cc, int ninp ) const {
        return ninp ? tb : ta;
    }
    Type *tr;
    Type *ta;
    Type *tb;
};

template<class OP>
Ptr<Inst> _op( Type *tr, Type *ta, Ptr<Inst> a, Type *tb, Ptr<Inst> b, OP ) {
    Op<OP> *res = new Op<OP>( tr, ta, tb );
    res->add_inp( a );
    res->add_inp( b );
    return res;
}

Ptr<Inst> op( Type *tr, Type *ta, Ptr<Inst> a, Type *tb, Ptr<Inst> b, Op_add o ) {
    return _op( tr, ta, a, tb, b, o );
}
