#ifndef SLICE_H
#define SLICE_H

#include "Inst.h"

Expr slice( Expr ptr, Type *off_type, Expr off, int len ); ///< with an initial value
Expr slice( Expr ptr, int off, int len ); ///< helper


#endif // SLICE_H
