#include <Stela/System/ReadFile.h>
#include <Stela/Met/Lexer.h>
#include <Stela/Ast/Ast.h>

int main() {
    ReadFile r( "tests/test.met" );

    // -> lexical data
    ErrorList error_list;
    Lexer l( error_list );
    l.parse( r.data, "tests/test.met" );
    if ( error_list )
        return 1;

    //
    Ast *ast = make_ast( error_list, l.root(), true );
    PRINTN( *ast );
}
