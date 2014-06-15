#include "../System/UsualStrings.h"
#include "../Ir/CallableFlags.h"
#include "Class.h"
#include "Slice.h"
#include "Type.h"
#include "Def.h"
#include "Cst.h"
#include "Ip.h"
#include "Op.h"

Def::TrialDef::TrialDef( Def *orig ) : orig( orig ) {
}

Def::TrialDef::~TrialDef() {
}

Expr Def::TrialDef::call( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, int apply_mode, Scope *caller, const BoolOpSeq &cond, Expr catched_vars ) {
    if ( apply_mode != Scope::APPLY_MODE_STD )
        TODO;

    // new scope (with local static variables)
    String path = "Def_" + to_string( ip->str_cor.str( orig->name ) ) + "_" + orig->sf->name + "_" + to_string( orig->off );
    for( int i = 0; i < orig->arg_names.size(); ++i )
        path += "_" + to_string( *args[ i ]->type() );
    Scope ns( &ip->main_scope, caller, path );
    ns.callable = orig;
    ns.cond = ns.cond and cond;
    ns.local_vars = args;

    // particular case
    if ( orig->name == STRING_init_NUM ) {
        // get self
        Type *vt = catched_vars->ptype();
        Expr self;
        for( int i = 0, o = 0; ; ++i ) {
            if ( i == orig->catched_vars.size() )
                return ip->ret_error( "no self ??" );
            if ( orig->catched_vars[ i ].type == IN_SELF ) {
                Type *ts = ip->type_from_type_var( vt->parameters[ 0 ] );
                self = slice( ts, catched_vars->get( cond ), o );
                break;
            }
            vt = ip->type_from_type_var( vt->parameters[ 2 ] );
            o += ip->type_ST->size();
        }

        // init attributes
        Type *self_type = self->ptype();
        if ( self_type->orig->ancestors.size() )
            TODO;
        Expr o = 0;
        for( Class::Attribute &a : self_type->orig->attributes ) {
            Expr val = ns.parse( a.code.sf, a.code.tok, "arg init" );
            if ( a.type == CALLABLE_ATTR_TYPE ) // ~=
                TODO;
            ns.apply( ns.get_attr( rcast( val->type(), add( self, o ) ), STRING_reassign_NUM ), 1, &val );
            if ( not self_type->defined )
                self_type->attr_ptr_types << val->type();

            o = add( o, val->size() );
        }
        self_type->defined = true;
    }

    // inline call
    return ns.parse( orig->block.sf, orig->block.tok, "calling" );
}

Def::Def() {
}

void Def::read_bin( Scope *scope, BinStreamReader &bin ) {
    Callable::read_bin( scope, bin );

    if ( flags & IR_HAS_RETURN_TYPE ) {
        if ( name == STRING_init_NUM ) {
            int nb_args = bin.read_positive_integer();
            attr_init.reserve( nb_args );
            for( int n = 0; n < nb_args; ++n ) {
                AttrInit *ai = attr_init.push_back();
                ai->name = scope->read_nstring( bin );
                ai->nu = bin.read_positive_integer();
                for( int i = 0; i < ai->nu; ++i )
                    ai->args.push_back( sf, bin.read_offset() );
                ai->nn = bin.read_positive_integer();
                for( int i = 0; i < ai->nn; ++i ) {
                    ai->args.push_back( sf, bin.read_offset() );
                    ai->names.push_back( scope->read_nstring( bin ) );
                }
            }
        } else
            return_type = Code( sf, bin.read_offset() );
    }

    get_of = flags & IR_IS_A_GET ? scope->read_nstring( bin ) : -1;
    set_of = flags & IR_IS_A_SET ? scope->read_nstring( bin ) : -1;
    sop_of = flags & IR_IS_A_SOP ? scope->read_nstring( bin ) : -1;
}

Callable::Trial *Def::test( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, Scope *caller ) {
    TrialDef *res = new TrialDef( this );

    if ( flags & IR_HAS_COMPUTED_PERT ) return res->wr( "TODO: computed pertinence" );

    // nb arguments
    if ( pnu + pnn + nu + nn < min_nb_args() ) return res->wr( "no enough arguments" );
    if ( pnu + pnn + nu + nn > max_nb_args() ) return res->wr( "To much arguments" );

    Scope scope( &ip->main_scope, caller, "TrialDef_" + ip->str_cor.str( name ) + "_" + sf->name + "_" + to_string( off ) );

    if ( pnu + pnn )
        TODO;
    if ( has_varargs() ) {
        TODO; // var ordering

        //        Vec<Expr> v_args;
        //        Vec<int> v_names;
        //        for( int i = 0; i < nn; ++i ) {
        //            int o = arg_names.first_index_equal_to( names[ i ] );
        //            if ( o >= 0 ) {
        //                if ( arg_ok[ o ] )
        //                    ip->disp_error( "arg is already assigned", true );
        //                scope.local_vars << vn[ i ];
        //                arg_ok[ o ] = true;
        //            } else {
        //                v_args  << vn   [ i ];
        //                v_names << names[ i ];
        //            }
        //        }

        //        for( int i = 0; i < arg_names.size(); ++i ) {
        //            if ( arg_ok[ i ] )
        //                ip->disp_error( "arg is already assigned", true );
        //            scope.local_vars << vu[ i ];
        //            arg_ok[ i ] = true;
        //        }
        //        for( int i = arg_names.size(); i < nu; ++i )
        //            v_args << vu[ i ];


        //        Expr varargs = ip->make_Varargs( v_args, v_names );
        //        scope.reg_var( STRING_varargs_NUM, varargs );
    } else {
        // unnamed args
        for( int i = 0; i < nu; ++i )
            scope.local_vars << vu[ i ];
        // basic check
        for( int n = 0; n < nn; ++n )
            if ( not arg_names.contains( names[ n ] ) )
                return res->wr( "named argument that does not appear in def args" );
        // named args
        Vec<bool> used_arg( Size(), nn, false );
        for( int i = nu; i < arg_names.size(); ++i ) {
            int arg_name = arg_names[ i ];
            for( int n = 0; ; ++n ) {
                if ( n == nn ) {
                    // not specified arg
                    int j = i - ( arg_names.size() - arg_defaults.size() );
                    if ( j < 0 )
                        return res->wr( "unspecified mandatory argument" );
                    scope.local_vars << scope.parse( arg_defaults[ j ].sf, arg_defaults[ j ].tok, "making default value" );
                    break;
                }
                if ( arg_name == names[ n ] ) {
                    scope.local_vars << vn[ n ];
                    used_arg[ n ] = true;
                    break;
                }
            }
        }
        for( int n = 0; n < nn; ++n )
            if ( not used_arg[ n ] )
                ip->disp_error( "arg assigned twice", true );
    }

    // arg constraints
    for( int i = 0; i < arg_constraints.size(); ++i ) {
        Expr v = scope.local_vars[ i ];
        int n = v->type()->orig->name;
        if ( int t = arg_constraints[ i ].class_names.size() ) {
            for( int j = 0; ; ++j ) {
                if ( j == t )
                    return res->wr( "type constraint not fullfilled" );
                if ( arg_constraints[ i ].class_names[ j ] == n )
                    break;
            }
        }
    }

    // condition
    if ( condition ) {
        TODO;
        //res->cond = scope.parse( condition.sf, condition.tok, "evaluating condition" )->get( scope.cond );
        if ( res->cond.always( false ) )
            return res->wr( "condition = false" );
    }

    res->args = scope.local_vars;
    return res;
}
