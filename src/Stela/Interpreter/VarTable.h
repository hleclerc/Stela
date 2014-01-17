#ifndef VARTABLE_H
#define VARTABLE_H

#include "../System/SplittedVec.h"
#include "Var.h"

/**
*/
class VarTable : public ObjectWithCptUse {
public:
    struct SV {
        int name;
        Var var;
    };

    VarTable( VarTable *parent = 0 );
    Var get( int name );
    void get( Vec<Var> &res, int name );
    void reg( int name, Var var );

    SplittedVec<SV,8> lst;
    VarTable *parent; ///< used only for static var tables
};

#endif // VARTABLE_H
