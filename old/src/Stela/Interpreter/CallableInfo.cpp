#include "CallableInfo.h"
#include "Interpreter.h"
#include "Scope.h"


// CallableInfo::Trial
CallableInfo::Trial::Trial( const char *reason ) : reason( reason ) {
}

CallableInfo::Trial::~Trial() {
}

Var CallableInfo::Trial::call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var &self, const Expr &sf, int off, Scope *caller, int apply_mode ) {
    ERROR( "weird... should not be here" );
    return ip->error_var;
}

// CallableInfo
CallableInfo::~CallableInfo() {
}

