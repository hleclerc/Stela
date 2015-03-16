#ifndef CONCAT_H
#define CONCAT_H

#include "Expr.h"

/// concatenate the bits of a and b
Expr concat( Expr a, Expr b );
Expr concat( Expr a, Expr b, Expr c );
Expr concat( Expr a, Expr b, Expr c, Expr d );

#endif // CONCAT_H
