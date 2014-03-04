#include "OpStructs.h"

#define DECL_IR_TOK( OP ) const char *Op_##OP::name() { return #OP; }
#include "../Ir/Decl_Operations.h"
#undef DECL_IR_TOK
