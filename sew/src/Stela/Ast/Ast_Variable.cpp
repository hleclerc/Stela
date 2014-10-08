#include "../Ir/Numbers.h"
#include "Ast_Variable.h"
#include "IrWriter.h"

#include "../Conv/ConvContext.h"
#include "../Conv/ConvScope.h"

Ast_Variable::Ast_Variable( int off, String str ) : Ast( off ), str( str ) {
}

void Ast_Variable::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    if ( not avail.count( str ) )
        res.insert( str );
}

void Ast_Variable::write_to_stream( Stream &os, int nsp ) const {
    os << str;
}

//void IrWriter::parse_variable( const Lexem *l ) {
//    if ( l->eq( "true" ) ) {
//        data << IR_TOK_TRUE;
//        push_offset( l );
//    } else if ( l->eq( "false" ) ) {
//        data << IR_TOK_FALSE;
//        push_offset( l );
//    //} else if ( l->eq( "self" ) ) {
//    //    data << IR_TOK_SELF;
//    //    push_offset( l );
//    } else if ( l->eq( "this" ) ) {
//        data << IR_TOK_THIS;
//        push_offset( l );
//    } else if ( l->eq( "continue" ) ) {
//        data << IR_TOK_CONTINUE;
//        push_offset( l );
//        data << 1; // nb of loop to continue
//    } else if ( l->eq( "break" ) ) {
//        data << IR_TOK_BREAK;
//        push_offset( l );
//        data << 1; // nb of loop to break
//    } else if ( l->eq( "null_ref" ) ) {
//        data << IR_TOK_NULL_REF;
//        push_offset( l );
//    } else {
//        Vec<CatchedVar> cvl;
//        find_needed_var( cvl, l );
//        if ( cvl.size() ) {
//            int r = 0;
//            bool surdef = false;
//            for( CatchedVar cv : cvl ) {
//                if ( cv.l->num_scope ) {
//                    surdef |= cv.surdef;
//                    ++r;
//                }
//            }
//            if ( surdef ) {
//                data << IR_TOK_VAR_SET;
//                push_offset( l );
//                data << r;
//                push_nstring( l );
//            }
//            for( CatchedVar cv : cvl ) {
//                if ( cv.l->num_scope ) {
//                    if ( cv.s >= 0 ) {
//                        // -> in catched vars of cv.l
//                        data << IR_TOK_VAR_IN_CATCHED_VARS;
//                        if ( not surdef )
//                            push_offset( l );
//                        data << cv.s;
//                    } else if ( cv.l->attribute ) {
//                        data << IR_TOK_VAR_IN_ATTR;
//                        if ( not surdef )
//                            push_offset( l );
//                        push_nstring( cv.l );
//                    } else {
//                        // -> in local or static scope
//                        if ( cv.l->scope_type & Lexem::SCOPE_TYPE_CLASS )
//                            TODO;
//                        data << ( cv.l->scope_type & Lexem::SCOPE_TYPE_STATIC ? IR_TOK_VAR_IN_STATIC_SCOPE : IR_TOK_VAR_IN_LOCAL_SCOPE );
//                        if ( not surdef )
//                            push_offset( l );
//                        data << l->num_scope - cv.l->num_scope;
//                        data << cv.l->num_in_scope;
//                    }
//                    if ( not surdef )
//                        return;
//                }
//            }
//            if ( surdef )
//                return;
//        }

//        data << PI8( IR_TOK_VAR );
//        push_offset( l );
//        push_nstring( l );
//    }
//}

void Ast_Variable::_get_info( IrWriter *aw ) const {
    aw->push_nstring( str );
}

PI8 Ast_Variable::_tok_number() const {
    return IR_TOK_VAR;
}

Past Ast_Variable::_parse_in( ConvScope &scope ) {
    if ( Past res = scope.find_var( str ) )
        return res;
    return cc->error_var();
}
