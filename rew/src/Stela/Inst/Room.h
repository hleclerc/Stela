#ifndef ROOM_H
#define ROOM_H

#include "Inst.h"

Expr room( int len, Expr val ); ///< with an initial value
Expr room( int len ); ///< default value (uninitialized memory)

#endif // ROOM_H
