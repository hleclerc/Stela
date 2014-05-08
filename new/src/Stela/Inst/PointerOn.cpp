#include "PointerOn.h"
#include "Ip.h"

/**
*/
class PointerOn : public Inst {
public:
    PointerOn( const Var &var ) : var( Ref(), var ) {
    }
    virtual void write_dot( Stream &os ) const {
        os << "&";
    }
    virtual void write_to_stream( Stream &os ) const {
        os << "&" << var;
    }
    virtual int size() const {
        return ip->type_RawPtr.size();
    }
    virtual Ptr<Inst> _pointer_on( int beg, int len ) {
        if ( beg == 0 and len == var.size() )
            return var.inst;
        return 0;
    }

    Var var;
};

Ptr<Inst> pointer_on( const Var &var ) {
    return new PointerOn( var );
}
