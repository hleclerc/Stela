#ifndef CODEGEN_H
#define CODEGEN_H

#include "../Inst/Inst.h"

/**
*/
class Codegen {
public:
    Codegen &operator<<( Expr inst );
    virtual void write_to( Stream &os ) = 0;
    virtual void exec() = 0;

protected:
    Vec<Expr> fresh;
};

#endif // CODEGEN_H
