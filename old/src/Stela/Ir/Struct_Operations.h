#ifndef STRUCT_OPERATIONS_H
#define STRUCT_OPERATIONS_H

#include "../System/Stream.h"

#define DECL_IR_TOK( OP ) \
    struct Op_##OP { enum { nb_args = 1 }; void write_to_stream( Stream &os ) const; };
#include "Decl_UnaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( OP ) \
    struct Op_##OP { enum { nb_args = 2 }; void write_to_stream( Stream &os ) const; };
#include "Decl_BinaryOperations.h"
#undef DECL_IR_TOK


#endif // STRUCT_OPERATIONS_H
