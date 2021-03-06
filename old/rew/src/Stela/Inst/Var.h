#ifndef VAR_H
#define VAR_H

#include "Inst.h"
class Type;

/**
  Reference (room) to an expression

*/
class Var {
public:
    enum {
        SURDEF = 1,
        WEAK_CONST = 2
    };

    Var( Ref, Type *type, Expr ptr ); ///<
    Var( Type *type, Expr val ); ///< reserve new room and set val to `val`
    Var( Type *type );
    Var();

    Var( SI64 val );

    void write_to_stream( Stream &os ) const;
    void set_val( int offset, Type *type, Expr val, Rese, Expr cond );
    void set_val( int offset, Var val, Rese, Expr cond );
    void set_val( Expr val, Rese, Expr cond );
    void set_val( Var val, Rese, Expr cond );
    Expr get_val();
    Expr ref();

    bool get_val( SI32 &val );

    bool always( Bool val );

    bool is_surdef() const;
    bool is_weak_const() const;
    bool is_an_error() const;
    bool defined() const;

    Var ptr();
    Var at( Type *type );
    Var conv( Type *type );
    Var operator&&( Var b );
    Var operator||( Var b );
    Var operator!();

    bool operator==( Var b );
    bool operator!=( Var b );

    Var operator+( Var b );

    friend Var syscall( const Vec<Var> &inp );

    int cpt_use() const { return inst->cpt_use; }

    Type *type;
    Expr  inst; ///< Room or equivalent (e.g. Room+offset, ...)
    int   flags;
};

Var syscall( const Vec<Var> &inp );
Var copy( const Var &var );

#endif // VAR_H
