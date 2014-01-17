#ifndef CPPINSTCOMPILER_H
#define CPPINSTCOMPILER_H

#include "../Interpreter/InstVisitor.h"
#include "CppCompiler.h"

/**
*/
class CppInstCompiler : public InstVisitor {
public:
    typedef CppCompiler::Reg Reg;

    CppInstCompiler( CppCompiler *cc );

    virtual void operator()( const Inst    &inst );

    virtual void operator()( const Syscall &inst );
    virtual void operator()( const Rand    &inst );
    virtual void operator()( const Cst     &inst );

    CppCompiler *cc;
};

#endif // CPPINSTCOMPILER_H
