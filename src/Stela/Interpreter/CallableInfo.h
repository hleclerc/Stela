#ifndef CALLABLEINFO_H
#define CALLABLEINFO_H

#include "../System/Vec.h"
#include "Var.h"
class BinStreamReader;
class Interpreter;
class SourceFile;
class Scope;

/**
*/
class CallableInfo {
public:
    struct Trial {
        Trial( const char *reason = 0 );
        virtual ~Trial();

        virtual void call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var *sf, int off, Var &res, Expr cond, Scope *caller );
        Trial *wr( const char *r ) { reason = r; return this; }
        bool ok() const { return not reason; }

        Var           cond;
        CallableInfo *ci;
        const char   *reason;
    };

    struct Code {
        Code( const PI8 *sf = 0, const PI8 *tok = 0 ) : sf( sf ), tok( tok ) {}
        const PI8 *sf;
        const PI8 *tok;
    };

    virtual ~CallableInfo();
    virtual Trial *test( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var *sf, int off, Scope *caller ) = 0;
    virtual const char *filename() const = 0;
    virtual int off() const = 0;

    double pertinence;
};



#endif // CALLABLEINFO_H
