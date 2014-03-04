#ifndef OPSTRUCTS_H
#define OPSTRUCTS_H

enum {
#define DECL_IR_TOK( OP ) ID_##OP,
#include "../Ir/Decl_BinaryOperations.h"
#include "../Ir/Decl_UnaryOperations.h"
#undef DECL_IR_TOK
    ID_fake
};

#define DECL_IR_TOK( OP ) struct Op_##OP { enum { nb_ch = 2, op_id = ID_##OP }; static const char *name(); };
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( OP ) struct Op_##OP { enum { nb_ch = 1, op_id = ID_##OP }; static const char *name(); };
#include "../Ir/Decl_UnaryOperations.h"
#undef DECL_IR_TOK

#endif // OPSTRUCTS_H
