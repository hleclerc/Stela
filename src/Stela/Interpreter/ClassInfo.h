#ifndef ClassInfo_H
#define ClassInfo_H

#include "../System/BinStreamReader.h"
#include "CallableInfo_WT.h"
class TypeInfo;

/**
*/
class ClassInfo : public CallableInfo_WT {
public:
    struct TrialClass : Trial {
        virtual void call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Expr *sf, int off, Var &res, Expr cond, Scope *caller );
        ClassInfo *orig;
        Vec<Var> args;
    };

    ClassInfo( const Expr *sf, int src_off, BinStreamReader bin, Expr class_ptr );
    virtual ~ClassInfo();

    virtual Trial *test( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Expr *sf, int off, Scope *caller );

    Vec<Code> ancestors;
    Expr      class_ptr;
    TypeInfo *last;
};

#endif // ClassInfo_H
