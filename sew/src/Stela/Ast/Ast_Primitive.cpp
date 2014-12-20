#include "../Ssa/ParsingContext.h"
#include "../Ir/Numbers.h"
#include "Ast_Primitive.h"
#include "IrWriter.h"

Ast_Primitive::Ast_Primitive( int off, int tok_number ) : Ast( off ), tok_number( tok_number ) {
}

void Ast_Primitive::get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const {
    for( int i = 0; i < args.size(); ++i )
        args[ i ]->get_potentially_needed_ext_vars( res, avail );
}

void Ast_Primitive::write_to_stream( Stream &os, int nsp ) const {
    os << "Primitive";
}

void Ast_Primitive::_get_info( IrWriter *aw ) const {
    aw->data << args.size();
    for( int i = 0; i < args.size(); ++i )
        aw->push_delayed_parse( args[ i ].ptr() );
}

#define CHECK_NB_ARGS( N ) if ( a->args.size() != N ) return ip->pc->ret_error( "Expecting " #N " args" )

static Expr parse_info( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_disp( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_rand( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_syscall( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_set_base_size_and_alig( const Ast_Primitive *a ) {
    CHECK_NB_ARGS( 2 );
    TODO;
    return Expr();
}
static Expr parse_set_RawRef_dependancy ( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_reassign_rec( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_assign_rec( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_set_ptr_val( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_select_SurdefList( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_ptr_size( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_ptr_alig( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_size_of( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_alig_of( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_typeof( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_address( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_get_slice( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_pointed_value( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_pointer_on( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_block_exec( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_get_argc( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_get_argv( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_apply_LambdaFunc( const Ast_Primitive *a ) { TODO; return Expr(); }
static Expr parse_inst_of( const Ast_Primitive *a ) { TODO; return Expr(); }


Expr Ast_Primitive::_parse_in( ParsingContext &context ) const {
    switch ( tok_number ) {
    #define DECL_IR_TOK( N ) case IR_TOK_##N: return parse_##N( this );
    #include "../Ir/Decl_Primitives.h"
    #undef DECL_IR_TOK
    }
    return context.ret_error( "Not a primitive", false, __FILE__, __LINE__ );
}

PI8 Ast_Primitive::_tok_number() const {
    return tok_number;
}


