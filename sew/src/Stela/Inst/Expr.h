#ifndef EXPR_H
#define EXPR_H

#include "../System/Stream.h"
class BoolOpSeq;
class Inst;

/**
*/
class Expr {
public:
    Expr( const Expr &obj );
    Expr( Inst *inst );
    Expr( SI32 val ); ///< cst
    Expr( PI8 val ); ///< cst
    Expr(); ///< unitialised variable
    ~Expr();

    Expr &operator=( const Expr &obj );

    bool operator==( const Expr &expr ) const;
    bool operator!=( const Expr &expr ) const { return not operator==( expr ); }
    bool operator<( const Expr &expr ) const { return inst < expr.inst; }
    operator bool() const { return inst; }

    const Inst *operator->() const { return inst; }
    Inst *operator->() { return inst; }

    const Inst &operator*() const { return *inst; }
    Inst &operator*() { return *inst; }

    void write_to_stream( Stream &os ) const;

    Inst *inst;
};

#endif // EXPR_H
