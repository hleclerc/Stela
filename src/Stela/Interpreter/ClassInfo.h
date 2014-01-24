#ifndef ClassInfo_H
#define ClassInfo_H

#include "CallableInfo_WT.h"
class TypeInfo;

/**
*/
class ClassInfo : public CallableInfo_WT {
public:
    struct TrialClass : Trial {
        virtual void call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const PI8 *sf, int off, Var &res, Expr cond, Scope *caller );
        ClassInfo *orig;
        Vec<Var> args;
    };

    ClassInfo( Interpreter *ip, const PI8 *sf, const PI8 *tok_data, int src_off );
    virtual ~ClassInfo();

    virtual Trial *test( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const PI8 *sf, int off, Scope *caller );

    Vec<Code> ancestors;
    TypeInfo *last;
    Expr      expr;

};

#endif // ClassInfo_H
