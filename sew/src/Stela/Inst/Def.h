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
        virtual Expr call( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, int apply_mode, Scope *caller );
        Vec<Expr> args; ///< def arguments
        Def *orig;
    };

    struct AttrInit {
        SI32 name;
        SI32 nu, nn;
        Vec<Code> args;
        Vec<SI32> names; ///< named arguments
    };

    Def();
    virtual void read_bin( Scope *scope, BinStreamReader &bin );
    virtual Trial *test( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, Scope *caller );

    // data
    Code return_type;
    Vec<AttrInit> attr_init; ///< if init

    int get_of;
    int set_of;
    int sop_of;
};

#endif // DEF_H
