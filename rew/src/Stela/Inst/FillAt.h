#ifndef FILLAT_H
#define FILLAT_H

#include "Inst.h"

/// offset in bits (not in bytes)
Expr fill_at( Expr src, Expr val, Type *off_type, Expr off );

#endif // FILLAT_H
