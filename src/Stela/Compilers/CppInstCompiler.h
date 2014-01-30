#ifndef CPPINSTCOMPILER_H
#define CPPINSTCOMPILER_H

#include "../Inst/InstVisitor.h"
#include "CppCompiler.h"

/**
*/
class CppInstCompiler : public InstVisitor {
public:
    typedef CppCompiler::Reg Reg;

    CppInstCompiler( CppCompiler *cc );

    virtual void def( const Inst &inst );

    // add, sub, ...
    #define DECL_OP( OP ) virtual void OP( const Inst &inst );
    #include "../Inst/DeclOp.h"
    #undef DECL_OP

    virtual void concat    ( const Inst &inst );
    virtual void syscall   ( const Inst &inst );
    virtual void pointer_on( const Inst &inst );
    virtual void rand      ( const Inst &inst, int size );
    virtual void val_at    ( const Inst &inst, int beg, int end );
    virtual void slice     ( const Inst &inst, int beg, int end );
    virtual void cst       ( const Inst &inst, const PI8 *value, const PI8 *known, int size_in_bits );

    CppCompiler *cc;
};

#endif // CPPINSTCOMPILER_H
