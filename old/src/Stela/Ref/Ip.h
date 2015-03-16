#ifndef IP_H
#define IP_H

#include "../System/ErrorList.h"
#include "../System/NstrCor.h"
#include "Type.h"

/**
  Context for Ref operations
*/
class Ip {
public:
    Ip();

    // errors
    void disp_error( String msg, bool warn = false );
    Var  make_error( String msg, bool warn = false );
    ErrorList::Error &error_msg( String msg, bool warn = false );

    Var error_var();

    // base types
    #define DECL_BT( T ) Type type_##T;
    #include "../Inst/DeclArytTypes.h"
    #undef DECL_BT
    Type type_Error;

    const BaseType *type_for( Type *type );
    Type *type_for( const BaseType *type );
    Type *pointer_type( Type *type );

    // strings
    NstrCor   str_cor;

    // output
    ErrorList error_list;
};

extern Ip *ip; ///<

#endif // IP_H
