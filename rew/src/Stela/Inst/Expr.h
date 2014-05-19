#ifndef EXPR_H
#define EXPR_H

#include "../System/Stream.h"
class Inst;

/**
*/
class Expr {
public:
    Expr( const Expr &obj );
    Expr( Inst *inst = 0 );
    ~Expr();

    Expr &operator=( const Expr &obj );

    operator bool() const { return inst; }

    bool operator==( const Expr &expr ) const { return inst == expr.inst; }

    const Inst *ptr() const { return inst; }
    Inst *ptr() { return inst; }

    const Inst *operator->() const { return inst; }
    Inst *operator->() { return inst; }

    const Inst &operator*() const { return *inst; }
    Inst &operator*() { return *inst; }

    void write_to_stream( Stream &os ) const;

    Inst *inst;
};

Expr simplified( Expr val );

#endif // EXPR_H
