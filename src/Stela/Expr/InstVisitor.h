#ifndef EXPR_INSTVISITOR_H
#define EXPR_INSTVISITOR_H

#include "../System/Vec.h"
#include "OpStructs.h"

namespace Expr_NS {

template<class OP>
class Op;

/**
*/
struct InstVisitor {
    virtual void operator()( const class Inst &inst ) {} // default

    // add, sub, ...
    #define DECL_OP( OP ) virtual void operator()( const Op<Op_##OP> &inst ) { operator()( reinterpret_cast<const Inst &>( inst ) ); }
    #include "DeclOp.h"
    #undef DECL_OP

    virtual void operator()( const class Reassign &inst, int off ) { operator()( reinterpret_cast<const Inst &>( inst ) ); }
    virtual void operator()( const class Syscall  &inst, int ptr_size ) { operator()( reinterpret_cast<const Inst &>( inst ) ); }
    virtual void operator()( const class Ptr      &inst, int ptr_size ) { operator()( reinterpret_cast<const Inst &>( inst ) ); }
    virtual void operator()( const class Rand     &inst, int ptr_size ) { operator()( reinterpret_cast<const Inst &>( inst ) ); }
    virtual void operator()( const class Cst      &inst, const Vec<PI8> &data ) { operator()( reinterpret_cast<const Inst &>( inst ) ); }
};

}

#endif // EXPR_INSTVISITOR_H
