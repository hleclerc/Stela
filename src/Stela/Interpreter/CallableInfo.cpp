#include "CallableInfo.h"
#include "Interpreter.h"
#include "Scope.h"


// CallableInfo::Trial
CallableInfo::Trial::Trial( const char *reason ) : reason( reason ) {
}

CallableInfo::Trial::~Trial() {
}

void CallableInfo::Trial::call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Expr *sf, int off, Var &res, Expr cond, Scope *caller ) {
    Interpreter *ip = caller->interpreter();
    res = ip->error_var;
}

// CallableInfo
CallableInfo::~CallableInfo() {
}

