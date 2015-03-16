#ifndef STELA_INST_ReplBits_H
#define STELA_INST_ReplBits_H

#include "Inst.h"

Expr repl_bits( Expr src, Expr off, Expr val ); /// *( &src + off ) = val

#endif // STELA_INST_ReplBits_H

