#ifndef SELECTDEP_H
#define SELECTDEP_H

#include "Inst.h"

Expr select_dep( Expr cond, Expr ok_dep, Expr ko_dep );

#endif // SELECTDEP_H
