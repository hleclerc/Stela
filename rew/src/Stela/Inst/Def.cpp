#include "../System/UsualStrings.h"
#include "../Ir/CallableFlags.h"
#include "Def.h"
#include "Ip.h"

Def::Def() {
}

void Def::read_bin( BinStreamReader &bin ) {
    Callable::read_bin( bin );

    if ( flags & IR_HAS_RETURN_TYPE ) {
        if ( name == STRING_init_NUM ) {
            int nb_args = bin.read_positive_integer();
            attr_init.reserve( nb_args );
            for( int n = 0; n < nb_args; ++n ) {
                AttrInit *ai = attr_init.push_back();
                ai->name = ip->read_nstring( bin );
                ai->nu = bin.read_positive_integer();
                for( int i = 0; i < ai->nu; ++i )
                    ai->args.push_back( sf, bin.read_offset() );
                ai->nn = bin.read_positive_integer();
                for( int i = 0; i < ai->nn; ++i ) {
                    ai->args.push_back( sf, bin.read_offset() );
                    ai->names.push_back( ip->read_nstring( bin ) );
                }
            }
        } else
            return_type = Code( sf, bin.read_offset() );
    }

    get_of = flags & IR_IS_A_GET ? ip->read_nstring( bin ) : -1;
    set_of = flags & IR_IS_A_SET ? ip->read_nstring( bin ) : -1;
    sop_of = flags & IR_IS_A_SOP ? ip->read_nstring( bin ) : -1;
}

Callable::Trial *Def::test( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var &self ) {
    TODO;
    return 0;
//    TrialDef *res = new TrialDef( this );

//    if ( flags & IR_HAS_COMPUTED_PERT ) return res->wr( "TODO: computed pertinence" );

//    // nb arguments
//    if ( pnu + pnn + nu + nn < min_nb_args() ) return res->wr( "no enough arguments" );
//    if ( pnu + pnn + nu + nn > max_nb_args() ) return res->wr( "To much arguments" );

//    res->scope = new Scope( ip->main_scope, caller );
//    res->scope->instantiated_from_off = off;
//    res->scope->instantiated_from_sf  = sf;
//    if ( self )
//        res->scope->self = self;


//    Vec<bool> arg_ok( Size(), arg_names.size(), false );
//    if ( pnu + pnn )
//        TODO;
//    if ( has_varargs() ) {
//        Vec<Var> uv_args, nv_args;
//        Vec<int> nv_name;

//        for( int i = 0; i < arg_names.size(); ++i ) {
//            res->scope->reg_var( arg_names[ i ], vu[ i ], sf, off, false, true );
//            arg_ok[ i ] = true;
//        }
//        for( int i = arg_names.size(); i < nu; ++i )
//            uv_args << vu[ i ];

//        for( int i = 0; i < nn; ++i ) {
//            int o = arg_names.first_index_equal_to( names[ i ] );
//            if ( o >= 0 ) {
//                if ( arg_ok[ o ] )
//                    caller->disp_error( "arg is already assigned", sf, off, true );
//                res->scope->reg_var( names[ i ], vn[ i ], sf, off, false, true );
//                arg_ok[ o ] = true;
//            } else {
//                nv_args << vn   [ i ];
//                nv_name << names[ i ];
//            }
//        }

//        Var varargs = ip->make_varargs_var( uv_args, nv_args, nv_name );
//        res->scope->reg_var( STRING_varargs_NUM, varargs, sf, off, false, true );
//    } else {
//        for( int i = 0; i < nu; ++i ) {
//            res->scope->reg_var( arg_names[ i ], vu[ i ], sf, off, false, true );
//            arg_ok[ i ] = true;
//        }
//        for( int i = 0; i < nn; ++i ) {
//            int o = arg_names.first_index_equal_to( names[ i ] );
//            if ( o < 0 )
//                return res->wr( "name=... does not appear in def" );
//            if ( arg_ok[ o ] )
//                caller->disp_error( "arg is already assigned", sf, off );
//            res->scope->reg_var( names[ i ], vn[ i ], sf, off, false, true );
//            arg_ok[ o ] = true;
//        }
//    }

//    // default values
//    for( int i = 0; i < arg_ok.size(); ++i ) {
//        if ( not arg_ok[ i ] ) {
//            int j = i - ( arg_names.size() - arg_defaults.size() );
//            if ( j < 0 )
//                return res->wr( "bad num default arg (weird)" );
//            res->scope->reg_var( arg_names[ i ], res->scope->parse( arg_defaults[ j ].sf, arg_defaults[ j ].tok ), sf, off, false, true );
//            arg_ok[ i ] = true;
//        }
//    }

//    // arg constraints
//    for( int i = 0; i < arg_constraints.size(); ++i ) {
//        Var v = res->scope->find_var( arg_names[ i ] );
//        int n = v.class_info()->name;
//        if ( int t = arg_constraints[ i ].class_names.size() ) {
//            for( int j = 0; ; ++j ) {
//                if ( j == t )
//                    return res->wr( "type constraint not fullfilled" );
//                if ( arg_constraints[ i ].class_names[ j ] == n )
//                    break;
//            }
//        }
//    }

//    // condition
//    if ( condition ) {
//        res->cond = res->scope->parse( condition.sf, condition.tok );
//        if ( ip->to_bool( res->cond, condition.sf, condition.tok ) == 0 )
//            return res->wr( "condition = false" );
//    }

//    return res;
}
