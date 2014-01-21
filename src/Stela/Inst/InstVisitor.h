#ifndef EXPR_INSTVISITOR_H
#define EXPR_INSTVISITOR_H

#include "../System/Vec.h"
class BaseType;
class Inst;

/**
*/
struct InstVisitor {
    virtual void def( const Inst &inst ) {} // default

    // add, sub, ...
    #define DECL_OP( OP ) virtual void OP( const Inst &inst, const BaseType *bt ) { def( inst ); }
    #include "DeclOp.h"
    #undef DECL_OP

    virtual void concat    ( const Inst &inst ) { def( inst ); }
    virtual void syscall   ( const Inst &inst, int ptr_size ) { def( inst ); }
    virtual void val_at    ( const Inst &inst, int size ) { def( inst ); }
    virtual void slice     ( const Inst &inst, int beg, int end ) { def( inst ); }
    virtual void pointer_on( const Inst &inst, int ptr_size ) { def( inst ); }
    virtual void rand      ( const Inst &inst, int size ) { def( inst ); }
    virtual void cst       ( const Inst &inst, const Vec<PI8> &data ) { def( inst ); }
};

#endif // EXPR_INSTVISITOR_H
