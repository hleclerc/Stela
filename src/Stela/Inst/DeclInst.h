DECL_INST( Concat )
DECL_INST( SetVal )
DECL_INST( Cst )
DECL_INST( PointerOn )
DECL_INST( Slice )
DECL_INST( SliceUnk )
DECL_INST( ValAt )
DECL_INST( Rand )
DECL_INST( Syscall )
DECL_INST( Conv )
DECL_INST( Phi )
#define DECL_IR_TOK( OP ) DECL_INST( Op_##OP )
#include "../Ir/Decl_Operations.h"
#undef DECL_IR_TOK

DECL_INST( If )
DECL_INST( IfOut )
DECL_INST( IfInp )

DECL_INST( Unknown )
DECL_INST( WhileInp )
DECL_INST( WhileOut )
DECL_INST( WhileInst )
