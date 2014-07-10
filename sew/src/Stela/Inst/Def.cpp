#include "../System/UsualStrings.h"
#include "../Ir/CallableFlags.h"
#include "Class.h"
#include "Slice.h"
#include "Type.h"
#include "Def.h"
#include "Cst.h"
#include "Ip.h"
#include "Op.h"

Def::TrialDef::TrialDef( Def *orig, Scope *caller ) : orig( orig ),
    ns( &ip->main_scope, caller,
           "TrialDef_" + ip->str_cor.str( orig->name ) + "_" + orig->sf->name + "_" + to_string( orig->off ) ) {

    ns.catched_vars = &orig->catched_vars;
}

Def::TrialDef::~TrialDef() {
}

Def::Def() {
}

void Def::read_bin( Scope *scope, BinStreamReader &bin ) {
    Callable::read_bin( scope, bin );

    if ( flags & IR_HAS_RETURN_TYPE )
        return_type = Code( sf, bin.read_offset() );

    if ( name == STRING_init_NUM ) {
        int nb_args = bin.read_positive_integer();
        attr_init.reserve( nb_args );
        for( int n = 0; n < nb_args; ++n ) {
            AttrInit *ai = attr_init.push_back();
            ai->attr = scope->read_nstring( bin );
            ai->nu = bin.read_positive_integer();
            for( int i = 0; i < ai->nu; ++i )
                ai->args.push_back( sf, bin.read_offset() );
            ai->nn = bin.read_positive_integer();
            for( int i = 0; i < ai->nn; ++i ) {
                ai->names.push_back( scope->read_nstring( bin ) );
                ai->args.push_back( sf, bin.read_offset() );
            }
        }
    }

    get_of = flags & IR_IS_A_GET ? scope->read_nstring( bin ) : -1;
    set_of = flags & IR_IS_A_SET ? scope->read_nstring( bin ) : -1;
    sop_of = flags & IR_IS_A_SOP ? scope->read_nstring( bin ) : -1;
}

Callable::Trial *Def::test( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, Scope *caller, Expr self ) {
    TrialDef *res = new TrialDef( this, caller );
    if ( self ) {
        res->ns.reg_var( STRING_self_NUM, self );
        res->ns.self = self;
    }

    if ( flags & IR_HAS_COMPUTED_PERT ) return res->wr( "TODO: computed pertinence" );

    // nb arguments
    if ( pnu + pnn + nu + nn < min_nb_args() ) return res->wr( "no enough arguments" );
    if ( pnu + pnn + nu + nn > max_nb_args() ) return res->wr( "To much arguments" );

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
            res->ns.reg_var( arg_names[ i ], vu[ i ] );
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
                    Expr v = res->ns.parse( arg_defaults[ j ].sf, arg_defaults[ j ].tok, "default value" );
                    res->ns.reg_var( arg_name, v );
                    break;
                }
                if ( arg_name == names[ n ] ) {
                    res->ns.reg_var( arg_name, vn[ n ] );
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
        Expr v = res->ns.find_var( arg_names[ i ] );
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

    return res;
}

Expr Def::TrialDef::call( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, int apply_mode, Scope *caller, const BoolOpSeq &cond, Expr self ) {
    if ( apply_mode != Scope::APPLY_MODE_STD )
        TODO;

    //
    ns.cond = ns.cond and cond;

    // particular case
    if ( orig->name == STRING_init_NUM ) {
        // init attributes
        Type *self_type = self->ptype();

        if ( self_type->orig->ancestors.size() )
            TODO;

        Vec<Bool> initialised_args( Size(), self_type->attributes.size(), false );
        for( AttrInit &d : orig->attr_init ) {
            for( int i = 0; ; ++i ) {
                if ( i == self_type->attributes.size() )
                    return ip->ret_error( "no attribute " + ip->str_cor.str( d.attr ) + " in class" );

                Type::Attr &a = self_type->attributes[ i ];
                if ( d.attr == a.name ) {
                    // parse args
                    Vec<Expr> args;
                    int nu = d.args.size() - d.names.size();
                    for( Code &c : d.args )
                        args << ns.parse( c.sf, c.tok, "starts_with" );

                    //
                    ns.apply( ns.get_attr( rcast( a.val->type(), add( self, a.off ) ), STRING_init_NUM ),
                              nu, args.ptr(), d.names.size(), d.names.ptr(), args.ptr() + nu );
                    initialised_args[ i ] = true;
                    break;
                }
            }
        }

        int cpt = 0;
        for( Type::Attr &a : self_type->attributes )
            if ( a.off >= 0 and not initialised_args[ cpt ] )
                ns.apply( ns.get_attr( rcast( a.val->type(), add( self, a.off ) ), STRING_init_NUM ), not ( a.val->flags & Inst::PART_INST ), &a.val );
            ++cpt;
    }

    // inline call
    return ns.parse( orig->block.sf, orig->block.tok, "calling" );
}
