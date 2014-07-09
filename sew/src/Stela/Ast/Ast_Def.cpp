#include "../Ir/CallableFlags.h"
#include "../Ir/Numbers.h"
#include "AstWriter.h"
#include "Ast_Def.h"

Ast_Def::Ast_Def( int off ) : Ast_Callable( off ) {
}


void Ast_Def::_get_info( AstWriter *aw ) const {
    Ast_Callable::_get_info( aw );

    // return type
    if ( return_type )
        aw->push_delayed_parse( return_type.ptr() );

    if ( name == "init" ) {
        aw->data << starts_with.size();

        for( int i = 0; i < starts_with.size(); ++i ) {
            const StartsWith_Item &sw = starts_with[ i ];
            int nn = sw.names.size(), nu = sw.args.size() - sw.names.size();

            // attr name
            aw->push_nstring( sw.attr );

            // unnamed args
            aw->data << nu;
            for( int i = 0; i < nu; ++i )
                aw->push_delayed_parse( sw.args[ i ].ptr() );

            // named args
            aw->data << nu;
            for( int i = 0; i < nn; ++i ) {
                aw->push_nstring( sw.names[ i ] );
                aw->push_delayed_parse( sw.args[ nu + i ].ptr() );
            }
        }
    }

    // get set sop
    if ( get.size() ) aw->push_nstring( get );
    if ( set.size() ) aw->push_nstring( set );
    if ( sop.size() ) aw->push_nstring( sop );
}

PI8 Ast_Def::_tok_number() const {
    return IR_TOK_DEF;
}

int Ast_Def::_spec_flags() const {
    return bool( get.size()  ) * IR_IS_A_GET +
           bool( set.size()  ) * IR_IS_A_SET +
           bool( sop.size()  ) * IR_IS_A_SOP +
           bool( return_type ) * IR_HAS_RETURN_TYPE;
}
