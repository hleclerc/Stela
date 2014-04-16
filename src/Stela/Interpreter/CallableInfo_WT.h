#ifndef CALLABLEINFO_WT_H
#define CALLABLEINFO_WT_H

#include "CallableInfo.h"
class BinStreamReader;
class VarTable;

/**
*/
class CallableInfo_WT : public CallableInfo {
public:
    struct ArgCst {
        Vec<int> class_names;
    };

    CallableInfo_WT( const Expr &sf, int src_off, BinStreamReader &bin, VarTable *sn );
    virtual const char *filename() const;
    virtual int off() const;

    int  min_nb_args() const;
    int  max_nb_args() const;
    int  nb_arg_vals() const;
    bool has_varargs() const;
    bool self_as_arg() const;

    Expr        sf;
    int         src_off;

    int         name;
    int         flags;
    int         nargs;
    int         dargs;
    Vec<int >   arg_names;
    Vec<Code>   arg_defaults;
    Vec<ArgCst> arg_constraints; ///< class names
    Code        comp_pert;
    Code        condition;
    Code        block;

    //
    VarTable *sn;
};

#endif // CALLABLEINFO_WT_H
