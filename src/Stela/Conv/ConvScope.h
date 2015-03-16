#ifndef CONVSCOPE_H
#define CONVSCOPE_H

#include "../System/Stream.h"
#include "../System/Vec.h"
#include "../Ast/Ast.h"

/**
*/
class ConvScope {
public:
    struct NamedVar {
        String name;
        Past   expr;
        bool   stat;
    };

    ConvScope( ConvScope *parent = 0 );

    bool reg_var( String name, Past expr, bool check_existing, bool stat );
    Past find_var( String name );

    Vec<NamedVar> variables;
    ConvScope    *parent;
    Past          base_size;
    Past          base_alig;
};

#endif // CONVSCOPE_H
