#include "Class.h"
#include "Type.h"

Class::Class() {
}

Type *Class::find_type( const Vec<Var> &args ) {
    for( Type *t : types )
        if ( t->parameters == args )
            return t;
    Type *res = new Type( name );
    res->orig = this;
    return res;
}
