#ifndef DEF_H
#define DEF_H

#include "Callable.h"
#include "Scope.h"

/**
*/
class Def : public Callable {
public:
    struct TrialDef : Trial {
        TrialDef( Def *orig, Scope *caller );
        virtual ~TrialDef();
        virtual Expr call( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, int apply_mode, Scope *caller, const BoolOpSeq &cond, Expr self );
        Def  *orig;
        Scope ns;
    };

    struct AttrInit {
        SI32 attr; ///< attribute name
        SI32 nu, nn;
        Vec<Code> args;
        Vec<SI32> names; ///< named arguments
    };

    Def();
    virtual void read_bin( Scope *scope, BinStreamReader &bin );
    virtual Trial *test( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, Scope *caller, Expr self );

    // data
    Code          return_type;
    Vec<AttrInit> attr_init; ///< if init

    int           get_of;
    int           set_of;
    int           sop_of;
};

#endif // DEF_H
