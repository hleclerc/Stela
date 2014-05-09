#ifndef CODEGEN_H
#define CODEGEN_H

#include "../Inst/Inst.h"

/**
*/
class CodeGen {
public:
    CodeGen();

    CodeGen &operator<<( ConstPtr<Inst> inst );
    virtual void write_to( Stream &os ) = 0;
    virtual void exec() = 0;

protected:
    Vec<ConstPtr<Inst> > computed;
    Vec<ConstPtr<Inst> > fresh;
};

#endif // CODEGEN_H
