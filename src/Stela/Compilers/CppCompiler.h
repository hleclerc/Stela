#ifndef CPPCOMPILER_H
#define CPPCOMPILER_H

#include "../Interpreter/Expr.h"

/**
*/
class CppCompiler {
public:
    CppCompiler();

    CppCompiler &operator<<( ConstPtr<Inst> inst );
    void exec();

protected:
    void get_front_rec( Vec<const Inst *> &front, const Inst *inst );

    Vec<ConstPtr<Inst> > outputs;
};

#endif // CPPCOMPILER_H
