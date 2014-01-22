#ifndef CALLABLEINFO_WT_H
#define CALLABLEINFO_WT_H

#include "CallableInfo.h"

/**
*/
class CallableInfo_WT : public CallableInfo {
public:
    CallableInfo_WT( const PI8 *sf, int src_off );
    void parse_wt( Interpreter *ip, const PI8 *sf, BinStreamReader &bin );
    virtual const char *filename() const;
    virtual int off() const;

    int min_nb_args() const;
    int max_nb_args() const;
    bool has_varargs() const;
    bool self_as_arg() const;

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

#endif // CALLABLEINFO_WT_H
