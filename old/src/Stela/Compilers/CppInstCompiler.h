#ifndef CPPINSTCOMPILER_H
#define CPPINSTCOMPILER_H

#include "../Inst/InstVisitor.h"
#include "CppCompiler.h"

/**
*/
class CppInstCompiler : public InstVisitor {
public:
    CppInstCompiler( CppCompiler *cc, bool inline_inst = false );
    bool decl( const Inst &inst, int nout ) const; ///< return true if decl should be done

    virtual void def( const Inst &inst );

    // add, sub, ...
    #define DECL_IR_TOK( OP ) virtual void op_##OP( const Inst &inst );
    #include "../Ir/Decl_Operations.h"
    #undef DECL_IR_TOK

    virtual void phi       ( const Inst &inst );
    virtual void concat    ( const Inst &inst );
    virtual void syscall   ( const Inst &inst );
    virtual void pointer_on( const Inst &inst );
    virtual void rand      ( const Inst &inst, int size );
    virtual void val_at    ( const Inst &inst, int beg, int end );
    virtual void slice     ( const Inst &inst, int beg, int end );
    virtual void conv      ( const Inst &inst, const BaseType *dst, const BaseType *src );
    virtual void cst       ( const Inst &inst, const PI8 *value, const PI8 *known, int size_in_bits );


    CppCompiler *cc;
    bool         inline_inst;
};

#endif // CPPINSTCOMPILER_H
