#include "../Inst/BaseType.h"
#include "Ip.h"

Ip::Ip() {
    #define DECL_BT( T ) type_##T.bt = bt_##T;
    #include "../Inst/DeclArytTypes.h"
    #undef DECL_BT

    #define DECL_BT( T ) type_##T.add_room( bt_##T->size_in_bits() );
    #include "../Inst/DeclArytTypes.h"
    #undef DECL_BT

    type_Error.add_room( 0 );
}

void Ip::disp_error( String msg, bool warn ) {
    std::cerr << error_msg( msg, warn );
}

Var Ip::make_error( String msg, bool warn ) {
    disp_error( msg, warn );
    return error_var();
}

ErrorList::Error &Ip::error_msg( String msg, bool warn ) {
    ErrorList::Error &res = error_list.add( msg, warn );
    //    if ( sf )
    //        res.ac( sf_info( sf )->filename, off );
    //    for( Scope *s = sc; s; s = s->caller )
    //        if ( s->instantiated_from_sf )
    //            res.ac( sf_info( s->instantiated_from_sf )->filename, s->instantiated_from_off );
    return res;
}

Var Ip::error_var() {
    return Var( &type_Error );
}

const BaseType *Ip::type_for( Type *type ) {
    #define DECL_BT( T ) if ( type == &type_##T ) return bt_##T;
    #include "../Inst/DeclArytTypes.h"
    #undef DECL_BT
    return 0;
}

Type *Ip::type_for( const BaseType *type ) {
    #define DECL_BT( T ) if ( type == bt_##T ) return &type_##T;
    #include "../Inst/DeclArytTypes.h"
    #undef DECL_BT
    return 0;
}

Type *Ip::pointer_type( Type *type ) {
    return 0;
}

Ip ip_inst; ///< default global ip
Ip *ip = &ip_inst; ///<
