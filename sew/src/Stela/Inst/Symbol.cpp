#include "Symbol.h"

/**
*/
struct Symbol : Inst {
    Symbol( Type *type, String name ) : out_type( type ), name( name ) {}
    virtual void write_dot( Stream &os ) { os << name; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Symbol( out_type, name ); }
    virtual Type *type() { return out_type; }
    Type *out_type;
    String name;
};

Expr symbol( Type *type, String name ) {
    Symbol *res = new Symbol( type, name );
    return res;
}

