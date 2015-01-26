#include "../Ssa/ParsingContext.h"
#include "../Ssa/SurdefList.h"
#include "../Ssa/Varargs.h"
#include "../Ssa/Symbol.h"
#include "../Ssa/Rcast.h"
#include "../Ssa/Room.h"
#include "../Ssa/Conv.h"
#include "../Ssa/Type.h"
#include "../Ssa/Cst.h"
#include "../Ssa/Op.h"
#include "../Ir/Numbers.h"
#include "../Codegen/TypeGen_JS.h"
#include "Ast_Primitive.h"
#include "IrWriter.h"

Ast_Primitive::Ast_Primitive( const char *src, int off, int tok_number ) : Ast( src, off ), tok_number( tok_number ) {
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

#define CHECK_NB_ARGS( N ) if ( p->args.size() != N ) return ip->pc->ret_error( "Expecting " #N " args" )

static Expr parse_info( ParsingContext &context, const Ast_Primitive *p ) {
    for( int i = 0; i < p->args.size(); ++i ) {
        Expr e = p->args[ i ]->parse_in( context );
        std::cout << e << " -> " << std::flush << e->get( context.cond ) << std::endl;
    }
    return Expr();
}
static Expr parse_disp( ParsingContext &context, const Ast_Primitive *p ) {
    Vec<Expr> var( Rese(), p->args.size() );
    for( Past arg : p->args )
        var << arg->parse_in( context )->get( context.cond );
    Inst::display_graph( var );
    return ip->void_var();
}
static Expr parse_rand( ParsingContext &context, const Ast_Primitive *p ) {
    static int num = 0;
    return room( symbol( ip->type_Bool, "rand(/*" + to_string( num++ ) + "*/)" ) );
}
static Expr parse_syscall( ParsingContext &context, const Ast_Primitive *p ) { TODO; return Expr(); }
static Expr parse_set_base_size_and_alig( ParsingContext &context, const Ast_Primitive *p ) {
    CHECK_NB_ARGS( 2 );
    Expr a = p->args[ 0 ]->parse_in( context )->get( context.cond );
    Expr b = p->args[ 1 ]->parse_in( context )->get( context.cond );
    if ( a.error() or b.error() )
        return a;
    if ( a->get_val( &context.base_size, ip->type_SI32 ) == false or b->get_val( &context.base_alig, ip->type_SI32 ) == false )
        return context.ret_error( "set_base_size_and_alig -> SI32/SI64 known values" );
    return ip->void_var();
}
static Expr parse_set_RawRef_dependancy ( ParsingContext &context, const Ast_Primitive *p ) { TODO; return Expr(); }

static Expr parse_reassign_rec( ParsingContext &context, const Ast_Primitive *p ) {
    CHECK_NB_ARGS( 2 );
    Expr dst = p->args[ 0 ]->parse_in( context );
    Expr src = p->args[ 1 ]->parse_in( context );
    Type *dt = dst->ptype();
    Type *st = src->ptype();
    if ( dt != st )
        dst->set( conv( dt, src->get( context.cond ) ), context.cond );
    else
        dst->set( src->get( context.cond ), context.cond );
    return dst;
}

static Expr parse_assign_rec( ParsingContext &context, const Ast_Primitive *p ) { TODO; return Expr(); }
static Expr parse_set_ptr_val( ParsingContext &context, const Ast_Primitive *p ) { TODO; return Expr(); }

static Expr parse_select_SurdefList( ParsingContext &context, const Ast_Primitive *p ) {
    CHECK_NB_ARGS( 2 );
    Expr self = p->args[ 0 ]->parse_in( context ); if ( self ) self = self->get( context.cond );
    Expr vara = p->args[ 1 ]->parse_in( context ); if ( vara ) vara = vara->get( context.cond );
    if ( self.error() or vara.error() )
        return Expr();
    SI64 psur, pvar;
    if ( self->get_val( &psur, 64 ) == false or vara->get_val( &pvar, 64 ) == false )
        return context.ret_error( "expecting cst values" );
    SurdefList *sur = reinterpret_cast<SurdefList *>( ST( psur ) );
    Varargs    *var = reinterpret_cast<Varargs    *>( ST( pvar ) );

    SurdefList *n_sur = new SurdefList( *sur );
    n_sur->parameters.append( *var );

    SI64 ptr = ST( n_sur );
    return room( cst( ip->type_SurdefList, 64, &ptr ) );
}

static Expr parse_select_Varargs( ParsingContext &context, const Ast_Primitive *p ) {
    CHECK_NB_ARGS( 2 );
    Expr self = p->args[ 0 ]->parse_in( context ); if ( self ) self = self->get( context.cond );
    Expr inde = p->args[ 1 ]->parse_in( context ); if ( inde ) inde = inde->get( context.cond );
    if ( self.error() or inde.error() )
        return Expr();
    SI64 psel, pind;
    if ( self->get_val( &psel, 64 ) == false or inde->get_val( &pind, ip->type_SI64 ) == false )
        return context.ret_error( "expecting cst values" );
    Varargs *var = reinterpret_cast<Varargs *>( ST( psel ) );
    if ( pind < 0 or pind >= var->exprs.size() )
        return ip->pc->ret_error( "index problem" );
    return var->exprs[ pind ];
}

static Expr parse_ptr_size( ParsingContext &context, const Ast_Primitive *p ) { TODO; return Expr(); }
static Expr parse_ptr_alig( ParsingContext &context, const Ast_Primitive *p ) { TODO; return Expr(); }

static Expr parse_size_of( ParsingContext &context, const Ast_Primitive *p ) {
    CHECK_NB_ARGS( 1 );
    Expr val = p->args[ 0 ]->parse_in( context );
    if ( val )
        val = val->get( context.cond );
    return val ? room( val->size() ) : val;
}

static Expr parse_alig_of( ParsingContext &context, const Ast_Primitive *p ) {
    CHECK_NB_ARGS( 1 );
    if ( Expr val = p->args[ 0 ]->parse_in( context ) )
        if ( Type *tp = val->ptype() )
            return room( tp->alig() );
    return Expr();
}

static Expr parse_typeof( ParsingContext &context, const Ast_Primitive *p ) {
    CHECK_NB_ARGS( 1 );
    Expr val = p->args[ 0 ]->parse_in( context );
    return context.type_expr( val->ptype() );
}

static Expr parse_address( ParsingContext &context, const Ast_Primitive *p ) { TODO; return Expr(); }
static Expr parse_get_slice( ParsingContext &context, const Ast_Primitive *p ) { TODO; return Expr(); }
static Expr parse_pointed_value( ParsingContext &context, const Ast_Primitive *p ) {
    CHECK_NB_ARGS( 1 );
    Expr val = p->args[ 0 ]->parse_in( context );
    // Expr off = p->args[ 1 ]->parse_in( context );
    // val = add( val, off->get( context.cond ) );
    return val ? val->get( context.cond ) : val;
}
static Expr parse_pointer_on( ParsingContext &context, const Ast_Primitive *p ) {
    CHECK_NB_ARGS( 1 );
    Expr val = p->args[ 0 ]->parse_in( context );
    return room( val );
}
static Expr parse_block_exec( ParsingContext &context, const Ast_Primitive *p ) { TODO; return Expr(); }
static Expr parse_get_argc( ParsingContext &context, const Ast_Primitive *p ) { TODO; return Expr(); }
static Expr parse_get_argv( ParsingContext &context, const Ast_Primitive *p ) { TODO; return Expr(); }
static Expr parse_apply_LambdaFunc( ParsingContext &context, const Ast_Primitive *p ) { TODO; return Expr(); }
static Expr parse_inst_of( ParsingContext &context, const Ast_Primitive *p ) { TODO; return Expr(); }
static Expr parse_repeat( ParsingContext &context, const Ast_Primitive *p ) {
    CHECK_NB_ARGS( 2 );
    Expr tvar = context.apply( p->args[ 0 ]->parse_in( context ), 0, 0, 0, 0, 0, ParsingContext::APPLY_MODE_PARTIAL_INST );
    Expr func = p->args[ 1 ]->parse_in( context ); ++func.inst->cpt_use;
    SI64 res[ 2 ];
    res[ 0 ] = ST( tvar->ptype() );
    res[ 1 ] = ST( func.inst );
    return room( cst( ip->type_Repeated, 2 * 64, res ) );
}
static Expr parse_code( ParsingContext &context, const Ast_Primitive *p ) {
    if ( p->args.size() == 0 )
        return ip->pc->ret_error( "Expecting at least one arg" );
    Expr f = p->args[ 0 ]->parse_in( context );
    PRINT( f );
    return Expr();
}
static Expr parse_repeated_type( ParsingContext &context, const Ast_Primitive *p ) {
    CHECK_NB_ARGS( 1 );
    Expr repv = p->args[ 0 ]->parse_in( context );
    if ( repv->ptype() != ip->type_Repeated )
        return context.ret_error( "Expecting a Repeated var" );
    SI64 rep_dat[ 2 ]; if ( not repv->get()->get_val( rep_dat, 2 * 64 ) ) ERROR( "..." );
    Type *rep_type = reinterpret_cast<Type *>( (ST)rep_dat[ 0 ] );
    return context.type_expr( rep_type );
}
static Expr parse_slice( ParsingContext &context, const Ast_Primitive *p ) {
    CHECK_NB_ARGS( 3 );
    Expr type = context.apply( p->args[ 0 ]->parse_in( context ), 0, 0, 0, 0, 0, ParsingContext::APPLY_MODE_PARTIAL_INST );
    Expr expr = p->args[ 1 ]->parse_in( context );
    Expr off  = p->args[ 2 ]->parse_in( context );
    if ( type.error() or expr.error() or off.error() )
        return Expr();
    expr = add( expr, off->get( context.cond ) );
    return rcast( context.ptr_type_for( type->ptype() ), expr );
}
static Expr parse_call( ParsingContext &context, const Ast_Primitive *p ) {
    CHECK_NB_ARGS( 2 );
    Expr func = p->args[ 0 ]->parse_in( context ); if ( func.error() ) return func;
    Expr args = p->args[ 1 ]->parse_in( context ); if ( args.error() ) return args;

    if ( args->ptype() != ip->type_Varargs )
        return context.ret_error( "size_init must return a varargs" );
    SI64 ptr;
    if ( not args->get( context.cond )->get_val( &ptr, 64 ) )
        return context.ret_error( "Expecting a known value" );
    Varargs *vargs = reinterpret_cast<Varargs *>( ST( ptr ) );

    return context.apply( func, vargs->nu(), vargs->ua(), vargs->nn(), vargs->ns(), vargs->na() );
}
static Expr parse_get_size( ParsingContext &context, const Ast_Primitive *p ) {
    CHECK_NB_ARGS( 1 );
    Expr self = p->args[ 0 ]->parse_in( context ); if ( self.error() ) return self;
    if ( self->ptype() != ip->type_Varargs )
        return context.ret_error( "get_size work only with varargs" );
    SI64 ptr;
    if ( not self->get( context.cond )->get_val( &ptr, 64 ) )
        return context.ret_error( "Expecting a known value" );
    Varargs *vargs = reinterpret_cast<Varargs *>( ST( ptr ) );
    return room( vargs->exprs.size() );
}

static Expr parse_make_code_for( ParsingContext &context, const Ast_Primitive *p ) {
    CHECK_NB_ARGS( 2 );
    Expr tvar = context.apply( p->args[ 0 ]->parse_in( context ), 0, 0, 0, 0, 0, ParsingContext::APPLY_MODE_PARTIAL_INST );
    Expr vara = p->args[ 1 ]->parse_in( context ); if ( vara ) vara = vara->get( context.cond );
    if ( tvar.error() or vara.error() )
        return Expr();

    SI64 pvar;
    if ( vara->get_val( &pvar, 64 ) == false )
        return context.ret_error( "expecting cst values" );
    Varargs *var = reinterpret_cast<Varargs *>( ST( pvar ) );

    TypeGen_JS type_gen( tvar->ptype() );
    for( int i = 0; i < var->nn(); ++i ) {
        Vec<Type *> types;
        PRINT( var->na()[ i ] );
        type_gen.add_func_to_gen( var->names[ i ], types );
    }
    type_gen.exec();

    return ip->void_var();
}


template<class OP>
static Expr parse_una( ParsingContext &context, const Ast_Primitive *p, OP o ) {
    CHECK_NB_ARGS( 1 );
    Expr a = p->args[ 0 ]->parse_in( context );
    return room( op( a->get( context.cond ), o ) );
}

template<class OP>
static Expr parse_bin( ParsingContext &context, const Ast_Primitive *p, OP o ) {
    CHECK_NB_ARGS( 2 );
    Expr a = p->args[ 0 ]->parse_in( context );
    Expr b = p->args[ 1 ]->parse_in( context );
    return room( op( a->get( context.cond ), b->get( context.cond ), o ) );
}

#define DECL_IR_TOK( N ) static Expr parse_##N( ParsingContext &context, const Ast_Primitive *p ) { return parse_una( context, p, Op_##N() ); }
#include "../Ir/Decl_UnaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( N ) static Expr parse_##N( ParsingContext &context, const Ast_Primitive *p ) { return parse_bin( context, p, Op_##N() ); }
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK


Expr Ast_Primitive::_parse_in( ParsingContext &context ) const {
    switch ( tok_number ) {
    #define DECL_IR_TOK( N ) case IR_TOK_##N: return parse_##N( context, this );
    #include "../Ir/Decl_Primitives.h"
    #include "../Ir/Decl_UnaryOperations.h"
    #include "../Ir/Decl_BinaryOperations.h"
    #undef DECL_IR_TOK
    }
    PRINT( tok_number );
    return context.ret_error( "Not a primitive", false, __FILE__, __LINE__ );
}

PI8 Ast_Primitive::_tok_number() const {
    return tok_number;
}


