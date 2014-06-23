#ifndef CPPREGCONSTRAINT_H
#define CPPREGCONSTRAINT_H

#include "Inst.h"
#include <map>

struct CppRegConstraint {
    enum {
        COMPULSORY = 100
    };
    struct Constraint {
        Expr expr;
        SI32 level; ///< in [0..COMPULSORY]
        bool equ; ///< true if must be equal, false if must be different
    };

    void add_equ( SI32 level, Expr expr_0, Expr expr_1 );
    void add_neq( SI32 level, Expr expr_0, Expr expr_1 );
    void add( SI32 level, Expr expr_0, Expr expr_1, bool equ );
    void write_to_stream( Stream &os ) const;

    const Vec<Constraint> *get_constraints_for( Expr expr );

    CppOutReg *compulsory_reg( Expr expr );

    std::map<Expr,Vec<Constraint> > constraints;
};


#endif // CPPREGCONSTRAINT_H
