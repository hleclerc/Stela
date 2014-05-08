#include "SetVal.h"

/**
*/
class SetVal : public Inst {
public:
    virtual void write_dot( Stream &os ) const {
        os << "set_val";
        if ( offset )
            os << "[off=" << offset << "]";
    }
    int offset;
};

Ptr<Inst> set_val( Ptr<Inst> src, Ptr<Inst> val, int offset ) {
    SetVal *res = new SetVal;
    res->offset = offset;
    res->add_inp( src );
    res->add_inp( val );
    return res;
}
