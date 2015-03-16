#include "Interpreter.h"
#include "VarTable.h"

VarTable::VarTable( Ptr<VarTable> parent ) : parent( parent ) {
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

void VarTable::write_to_stream( Stream &os ) const {
    for( int i = 0; i < lst.size(); ++i )
        os << ip->glob_nstr_cor.str( lst[ i ].name ) << " ";
}
