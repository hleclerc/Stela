#ifndef VAR_H
#define VAR_H

#include "Inst.h"
class Type;

/**
  Reference (room) to an expression

*/
class Var {
public:
    struct PVar {
        void write_to_stream( Stream &os ) const { os << var->get_val(); }
        void operator=( Expr expr ) { var->set_val( expr ); }
        void operator=( Var val ) { var->set_val( val ); }
        operator Expr() const { return var->get_val(); }
        Var *var;
    };

    Var( Ref, Type *type, Expr ptr ); ///<
    Var( Type *type, Expr val ); ///< reserve new room and set val to `val`
    Var( Type *type );

    void write_to_stream( Stream &os ) const;
    Expr operator*() const;
    PVar operator*();

    friend Var ptr( const Var &val );
    friend Var at( Type *type, Var ptr );
    friend Var and_boolean( Var a, Var b );


protected:
    friend struct PVar;
    Expr get_val() const;
    void set_val( Var val );
    void set_val( Expr val );

    Type *type;
    Expr  inst; ///< Room or equivalent (e.g. Room+offset, ...)
};

Var ptr( const Var &val ); ///< pointer on val
Var at( Type *type, Var ptr ); ///<
Var and_boolean( Var a, Var b );


#endif // VAR_H
