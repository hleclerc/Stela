#include "Codegen.h"

Codegen &Codegen::operator<<( Expr inst ) {
    fresh << inst;
    return *this;
}
