#ifndef CPPREGCONSTRAINT_H
#define CPPREGCONSTRAINT_H

#include "Inst.h"
#include <map>

struct CppRegConstraint {
    enum {
        COMPULSORY = 100
    };
    struct Plug {
        bool operator<( const Plug &p ) const { return expr.inst == p.expr.inst ? ninp < p.ninp : expr.inst < p.expr.inst; }
        Expr expr;
        SI32 ninp; ///< >= 0 for an input, -1 for the output
    };
    struct PlugWithLevel : Plug {
        SI32 level; ///< in [0..COMPULSORY]
        bool equ;
    };

    void add_equ( SI32 level, Expr expr_0, SI32 ninp_0, Expr expr_1, SI32 ninp_1 );
    void add_neq( SI32 level, Expr expr_0, SI32 ninp_0, Expr expr_1, SI32 ninp_1 );
    void add( SI32 level, Expr expr_0, SI32 ninp_0, Expr expr_1, SI32 ninp_1, bool equ );
    void write_to_stream( Stream &os ) const;

    const PlugWithLevel *get_out_constraint( Expr expr );
    const PlugWithLevel *get_inp_constraint( Expr expr, int ninp );

    CppOutReg *compulsory_reg( Expr expr );

    std::map<Plug,PlugWithLevel> constraints;
};


#endif // CPPREGCONSTRAINT_H
