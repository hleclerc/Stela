#ifndef REPLBITS_H
#define REPLBITS_H

#include "Inst.h"

Expr repl_bits( Expr src, Expr off, Expr val ); /// *( src + off ) = val, off is expressed in bits

#endif // REPLBITS_H
