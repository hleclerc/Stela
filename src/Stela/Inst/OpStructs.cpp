#include "OpStructs.h"

#define DECL_OP( OP ) const char *Op_##OP::name() { return #OP; }
#include "DeclOp.h"
#undef DECL_OP
