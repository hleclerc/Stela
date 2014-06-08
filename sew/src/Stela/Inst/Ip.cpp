#include "Class.h"
#include "Type.h"
#include "Cst.h"
#include "Ip.h"

Ip *ip;

Ip::Ip() {
    #define DECL_BT( T ) class_##T = new Class; class_##T->name = STRING_##T##_NUM;
    #include "DeclParmClass.h"
    #include "DeclBaseClass.h"
    #undef DECL_BT

    #define DECL_BT( T ) type_##T = new Type( class_##T );
    #include "DeclBaseClass.h"
    #undef DECL_BT

    #define DECL_BT( T ) type_##T->aryth = true;
    #include "DeclArytTypes.h"
    #undef DECL_BT

    type_SI32->_len = 32;
    type_SI64->_len = 64;

    type_ST = sizeof( void * ) == 8 ? type_SI64 : type_SI32;
}

Ip::~Ip() {
    #define DECL_BT( T ) delete type_##T;
    #include "DeclBaseClass.h"
    #undef DECL_BT

    #define DECL_BT( T ) delete class_##T;
    #include "DeclParmClass.h"
    #include "DeclBaseClass.h"
    #undef DECL_BT
}

Expr Ip::ret_error( String msg, bool warn, const char *file, int line ) {
    disp_error( msg, warn, file, line );
    return error_var();
}

void Ip::disp_error( String msg, bool warn, const char *file, int line ) {
    std::cerr << error_msg( msg, warn, file, line );
}

ErrorList::Error &Ip::error_msg( String msg, bool warn, const char *file, int line ) {
    ErrorList::Error &res = error_list.add( msg, warn );
    if ( file )
        res.caller_stack.push_back( line, file );

    if ( sf and off >= 0 )
        res.ac( sf->name.c_str(), off );
    for( int i = sf_stack.size() - 1; i >= 0; --i )
        if ( sf_stack[ i ].sf and sf_stack[ i ].off >= 0 )
            res.ac( sf_stack[ i ].sf->name.c_str(), sf_stack[ i ].off );
    return res;
}

Expr Ip::error_var() {
    return Expr( cst( type_Error, 0, 0 ) );
}
