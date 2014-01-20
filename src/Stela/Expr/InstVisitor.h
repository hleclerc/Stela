#ifndef EXPR_INSTVISITOR_H
#define EXPR_INSTVISITOR_H

#include "../System/Vec.h"
#include "OpStructs.h"

namespace Expr_NS {

class Inst;

template<class OP> class Op;
class Syscall;
class Rand;
class Ptr;
class Cst;

/**
*/
struct InstVisitor {
    virtual void operator()( const Inst &inst ) {} // default

    // add, sub, ...
    #define DECL_OP( OP ) virtual void operator()( const Op<Op_##OP> &inst ) { operator()( reinterpret_cast<const Inst &>( inst ) ); }
    #include "DeclOp.h"
    #undef DECL_OP

    virtual void operator()( const Syscall &inst, int ptr_size ) { operator()( reinterpret_cast<const Inst &>( inst ) ); }
    virtual void operator()( const Ptr     &inst, int ptr_size ) { operator()( reinterpret_cast<const Inst &>( inst ) ); }
    virtual void operator()( const Rand    &inst, int ptr_size ) { operator()( reinterpret_cast<const Inst &>( inst ) ); }
    virtual void operator()( const Cst     &inst, const Vec<PI8> &data ) { operator()( reinterpret_cast<const Inst &>( inst ) ); }
};

}

#endif // EXPR_INSTVISITOR_H
