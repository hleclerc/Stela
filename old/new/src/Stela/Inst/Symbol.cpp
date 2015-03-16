#include "../CodeGen/InstInfo_C.h"
#include "Symbol.h"

/**
*/
class Symbol : public Inst {
public:
    Symbol( String name, int len ) : name( name ), len( len ) {
    }
    virtual void write_dot( Stream &os ) const {
        os << name;
    }
    virtual int size() const {
        return len;
    }
    virtual Ptr<Inst> forced_clone( Vec<Ptr<Inst> > &created ) const {
        return new Symbol( name, len );
    }
    virtual void write_1l_to( CodeGen_C *cc ) const {
        *cc->os << name;
    }

    String name;
    int len;
};

Ptr<Inst> symbol( String name, int len ) {
    return new Symbol( name, len );
}
