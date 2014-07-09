import os, sys

def r( s ):
    return s.replace( "NAME", sys.argv[ 1 ] ).replace( "NLME", sys.argv[ 1 ].lower() )

h = r( """#ifndef STELA_AST_NAME_H
#define STELA_AST_NAME_H

#include "Ast.h"

/**
*/
class Ast_NAME : public Ast {
public:
    Ast_NAME( int off );
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;
    
protected:
    friend class AstMaker;
    
    virtual void _get_info( AstWriter *aw ) const;
    virtual PI8  _tok_number() const;
};

#endif // STELA_AST_NAME_H
""" )

c = r( """#include "Ast_NAME.h"

Ast_NAME::Ast_NAME( int off ) : Ast( off ) {
}

void Ast_NAME::write_to_stream( Stream &os, int nsp ) const {
    os << "NAME";
}

void Ast_NAME::_get_info( AstWriter *aw ) const {
    TODO;
}

PI8 Ast_NAME::_tok_number() const {
    return IR_TOK_NAME;
}
""" )


h_file = file( r( "src/Stela/Ast/Ast_NAME.h" ), "w" )
print >> h_file, h

c_file = file( r( "src/Stela/Ast/Ast_NAME.cpp" ), "w" )
print >> c_file, c

os.system( "./update_files.sh > /dev/null 2> /dev/null" )
os.system( r( "git add src/Stela/Ast/Ast_NAME.h src/Stela/Ast/Ast_NAME.cpp" ) )
