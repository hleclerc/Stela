#ifndef CALLABLEINFO_H
#define CALLABLEINFO_H

#include "../System/Vec.h"
class BinStreamReader;
class Interpreter;
class SourceFile;
class Var;

/**
*/
class CallableInfo {
public:
    struct Trial {
        Trial();

        CallableInfo *ci;
        const char   *reason; ///< 0 if OK
        double        pertinence;
    };

    struct Code {
        Code( const PI8 *sf = 0, const PI8 *tok = 0 ) : sf( sf ), tok( tok ) {}
        const PI8 *sf;
        const PI8 *tok;
    };

    virtual void test( Trial &trial, int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var *sf, int off ) = 0;
    virtual const char *filename() const = 0;
    virtual int off() const = 0;

    double pertinence;
};

class CallableInfo_WT : public CallableInfo {
public:
    CallableInfo_WT( const PI8 *sf, int src_off );
    void parse_wt( Interpreter *ip, const PI8 *sf, BinStreamReader &bin );
    virtual const char *filename() const;
    virtual int off() const;

    const PI8 *sf;
    int src_off;

    int       name;
    int       flags;
    int       nargs;
    int       dargs;
    Vec<int>  arg_names;
    Vec<Code> arg_defaults;
    Code      comp_pert;
    Code      condition;
    Code      block;
};

class CallableInfo_Class : public CallableInfo_WT {
public:
    CallableInfo_Class( Interpreter *ip, const PI8 *sf, const PI8 *tok_data, int src_off );
    virtual void test( Trial &trial, int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var *sf, int off );

    Vec<Code> ancestors;
};

class CallableInfo_Def : public CallableInfo_WT {
public:
    CallableInfo_Def( Interpreter *ip, const PI8 *sf, const PI8 *tok_data, int src_off );
    virtual void test( Trial &trial, int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var *sf, int off );

    Code block_with_ret;
    Code return_type;

    int get_of;
    int set_of;
    int sop_of;
};

#endif // CALLABLEINFO_H
