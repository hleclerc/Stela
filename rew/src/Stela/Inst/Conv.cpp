#include "Conv.h"

/**
*/
class Conv : public Inst {
public:
};

Expr conv( Type *dst, Type *src, Expr val ) {
    if ( dst == src )
        return val;
    TODO;
    return Expr();
}
