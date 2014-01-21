#ifndef OPSTRUCTS_H
#define OPSTRUCTS_H

#define ID_add 0

#define DECL_OP( OP ) struct Op_##OP { enum { nb_ch = 2, op_id = ID_##OP }; static const char *name(); };
#include "DeclOpBinary.h"
#undef DECL_OP

#define DECL_OP( OP ) struct Op_##OP { enum { nb_ch = 1, op_id = ID_##OP }; static const char *name(); };
#include "DeclOpUnary.h"
#undef DECL_OP

#endif // OPSTRUCTS_H
