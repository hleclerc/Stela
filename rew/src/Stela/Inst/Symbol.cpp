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
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new Symbol( name, len );
    }
    virtual int size() const {
        return len;
    }

    String name;
    int len;
};

Expr symbol( String name, int len ) {
    return new Symbol( name, len );
}
