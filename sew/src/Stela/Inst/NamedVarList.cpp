#include "NamedVarList.h"
#include "Ip.h"

Expr NamedVarList::add( int name, Expr expr ) {
    NamedVar *res = data.push_back();
    res->name = name;
    res->expr = expr;
    res->sf   = ip->cur_scope->sf;
    res->off  = ip->cur_scope->off;
    return res->expr;
}

bool NamedVarList::contains( int name ) {
    for( NamedVar &nv : data )
        if ( nv.name == name )
            return true;
    return false;
}

void NamedVarList::get( Vec<Expr> &lst, int name ) {
    for( NamedVar &nv : data )
        if ( nv.name == name )
            lst << nv.expr;
}

Expr NamedVarList::get( int name ) {
    for( NamedVar &nv : data )
        if ( nv.name == name )
            return nv.expr;
    return (Inst *)0;
}
