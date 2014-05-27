#ifndef VAR_H
#define VAR_H

#include "Inst.h"
class Type;

/**
  Reference (room) to an expression

*/
class Var {
public:
    Var( Ref, Type *type, Expr ptr ); ///<
    Var( Type *type, Expr val ); ///< reserve new room and set val to `val`
    Var( Type *type );
    Var();

    Var( SI64 val );

    void write_to_stream( Stream &os ) const;
    void set_val( Expr val );
    void set_val( Var val );
    Expr get_val();

    Var ptr();
    Var at( Type *type );
    Var operator&&( Var b );
    Var operator||( Var b );
    Var operator!();

    Var operator+( Var b );

    friend Var syscall( const Vec<Var> &inp );

protected:
    Type *type;
    Expr  inst; ///< Room or equivalent (e.g. Room+offset, ...)
};

Var syscall( const Vec<Var> &inp );

#endif // VAR_H
