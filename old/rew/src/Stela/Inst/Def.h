#ifndef DEF_H
#define DEF_H

#include "Callable.h"

/**
*/
class Def : public Callable {
public:
    struct TrialDef : Trial {
        TrialDef( Def *orig );
        virtual ~TrialDef();
        virtual Var call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var &self, int apply_mode );
        Scope *scope;
        Def *orig;
    };

    struct AttrInit {
        SI32 name;
        SI32 nu, nn;
        Vec<Code> args;
        Vec<SI32> names; ///< named arguments
    };

    Def();
    virtual void read_bin( BinStreamReader &bin );
    virtual Trial *test( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var &self );

    // data
    Code return_type;
    Vec<AttrInit> attr_init; ///< if init

    int get_of;
    int set_of;
    int sop_of;
};

#endif // DEF_H
