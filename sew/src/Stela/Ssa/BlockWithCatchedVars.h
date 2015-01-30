#ifndef BLOCKWITHCATCHEDVARS_H
#define BLOCKWITHCATCHEDVARS_H

#include "../Ast/Ast.h"
#include "Varargs.h"

/**
*/
class BlockWithCatchedVars {
public:
    BlockWithCatchedVars( Past block, ParsingContext *context, int rese_nn = 0 );

    Varargs         catched_vars;
    Vec<String>     name_args; ///< for blocks
    ParsingContext *context;
    Past            block;
};

#endif // BLOCKWITHCATCHEDVARS_H
