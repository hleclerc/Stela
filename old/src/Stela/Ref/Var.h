#ifndef VAR_H
#define VAR_H

#include "Ref.h"

/**
*/
class Var {
public:
    enum {
        WEAK_CONST = 1 // local constness (several Var can point on a single Ref)
    };


    Var( Type *type, PI8 *data = 0, PI8 *knwn = 0 );
    Var( SI32 val );
    Var();

    // operations
    friend Var pointed_val( const Var &var );
    friend Var pointer_on ( const Var &var );

    friend Var operator+( const Var &a, const Var &b );
    friend Var operator-( const Var &a, const Var &b );

    template<class Op>
    friend Var op( const Var &a, const Var &b, Op op_n, bool boolean );

    friend Var slice( const Var &a, Type *type, int offset );

    // output
    void write_to_stream( Stream &os ) const;

    Var  copy() const;
    Expr expr() const;

    // modifications
    void operator=( const Var &src );

    void operator+=( const Var &b );
    void operator-=( const Var &b );

    // information
    bool contains_a_ptr() const;
    bool weak_const() const;
    bool full_const() const;
    bool defined() const;

    Type *type() const;

protected:
    friend class RefItem_Slice;
    Var( Ref *ref, int flags = 0 );

    Ptr<Ref> ref;
    int      flags;
};

#endif // VAR_H
