#include "Struct_Operations.h"

#define DECL_IR_TOK( OP ) \
    void Op_##OP::write_to_stream( Stream &os ) const { os << #OP; }
#include "Decl_Operations.h"
#undef DECL_IR_TOK
