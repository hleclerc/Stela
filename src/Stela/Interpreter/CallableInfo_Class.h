#ifndef CALLABLEINFO_CLASS_H
#define CALLABLEINFO_CLASS_H

#include "CallableInfo_WT.h"

class CallableInfo_Class : public CallableInfo_WT {
public:
    struct TrialClass : Trial {
        virtual void call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const PI8 *sf, int off, Var &res, Expr cond, Scope *caller );
        CallableInfo_Class *orig;
        Vec<Var> args;
    };

    CallableInfo_Class( Interpreter *ip, const PI8 *sf, const PI8 *tok_data, int src_off );
    virtual Trial *test( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const PI8 *sf, int off, Scope *caller );

    Vec<Code> ancestors;
};

#endif // CALLABLEINFO_CLASS_H
