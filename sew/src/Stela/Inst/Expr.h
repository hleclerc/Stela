#ifndef EXPR_H
#define EXPR_H

#include "../System/Stream.h"
class Inst;

/**
*/
class Expr {
public:
    Expr( const Expr &obj );
    Expr( Inst *inst );
    Expr( SI32 val );
    Expr(); ///< unitialised variable
    ~Expr();

    Expr &operator=( const Expr &obj );

    const Inst *operator->() const { return inst; }
    Inst *operator->() { return inst; }

    const Inst &operator*() const { return *inst; }
    Inst &operator*() { return *inst; }


    void write_to_stream( Stream &os ) const;

    Inst *inst;
};

Expr simplified( Expr val );

#endif // EXPR_H
