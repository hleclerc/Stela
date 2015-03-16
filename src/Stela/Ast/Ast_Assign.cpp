#include "../Ssa/ParsingContext.h"
#include "../Ir/AssignFlags.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast_Assign.h"

Ast_Assign::Ast_Assign( const char *src, int off ) : Ast( src, off ) {
}

void Ast_Assign::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    val->get_potentially_needed_ext_vars( res, avail );
    avail.insert( name );
}

void Ast_Assign::prep_get_potentially_needed_ext_vars( std::set<String> &avail ) const {
    avail.insert( name );
}

void Ast_Assign::write_to_stream( Stream &os, int nsp ) const {
    os << name;
    if ( stat )
        os << "static ";
    if ( cons )
        os << "const ";
    if ( type )
        os << " ~= ";
    else
        os << " := ";
    if ( ref )
        os << "ref ";
    val->write_to_stream( os, nsp );
}

void Ast_Assign::_get_info( IrWriter *aw ) const {
    // name
    aw->push_nstring( name );

    // flags
    aw->data << ref  * IR_ASSIGN_REF +
                stat * IR_ASSIGN_STATIC +
                cons * IR_ASSIGN_CONST +
                type * IR_ASSIGN_TYPE;

    // val
    aw->push_delayed_parse( val.ptr() );
}

Expr Ast_Assign::_parse_in( ParsingContext &context ) const {
    // rhs
    Expr rhs = val->parse_in( context );
    if ( not rhs )
        return rhs;

    if ( type ) // ~=
        rhs = context.apply( rhs, 0, 0, 0, 0, 0, context.scope_type == ParsingContext::SCOPE_TYPE_CLASS ? ParsingContext::APPLY_MODE_PARTIAL_INST : ParsingContext::APPLY_MODE_STD );

    if ( ref ) // := ref
        TODO;

    if ( cons )
        TODO;

    if ( rhs->cpt_use > 1 )
        rhs = context.copy( rhs );

    context.reg_var( name, rhs, stat, _off, _src );
    return rhs;
}

PI8 Ast_Assign::_tok_number() const {
    return IR_TOK_ASSIGN;
}
