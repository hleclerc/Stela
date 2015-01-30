#include "BlockWithCatchedVars.h"

BlockWithCatchedVars::BlockWithCatchedVars( Past block, ParsingContext *context, int rese_nn ) : catched_vars( 0, rese_nn ), context( context ), block( block ) {
}
