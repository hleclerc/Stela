#include "PointerOn.h"
#include "Ip.h"

/**
*/
class PointerOn : public Inst {
public:
    PointerOn( const Var &var ) : var( var ) {
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
    virtual Ptr<Inst> forced_clone( Vec<Ptr<Inst> > &created ) const {
        var.inst->clone( created );
        return new PointerOn( Var( Ref(), var.type, reinterpret_cast<Inst *>( var.inst->op_mp ) ) );
    }

    Var var;
};

Ptr<Inst> pointer_on( const Var &var ) {
    return new PointerOn( var );
}
