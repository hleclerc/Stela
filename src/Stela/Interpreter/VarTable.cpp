#include "VarTable.h"

VarTable::VarTable( VarTable *parent ) : parent( parent ) {
}

Var VarTable::get( int name ) {
    for( int i = lst.size() - 1; i >= 0; --i )
        if ( lst[ i ].name == name )
            return lst[ i ].var;
    return Var();
}

void VarTable::get( Vec<Var> &res, int name ) {
    for( int i = lst.size() - 1; i >= 0; --i )
        if ( lst[ i ].name == name )
            res << lst[ i ].var;
}

void VarTable::reg( int name, Var var ) {
    lst << SV{ name, var };
}
