#ifndef CALLABLEINFO_DEF_H
#define CALLABLEINFO_DEF_H

#include "CallableInfo_WT.h"

class CallableInfo_Def : public CallableInfo_WT {
public:
    struct TrialDef : Trial {
        virtual void call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var *sf, int off, Var &res, Expr cond, Scope *caller );
    };

    CallableInfo_Def( Interpreter *ip, const PI8 *sf, const PI8 *tok_data, int src_off );
    virtual Trial *test( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var *sf, int off, Scope *caller );

    Code block_with_ret;
    Code return_type;

    int get_of;
    int set_of;
    int sop_of;
};

#endif // CALLABLEINFO_DEF_H
