#include "../System/UsualStrings.h"
#include "../Ir/CallableFlags.h"
#include "Def.h"
#include "Cst.h"
#include "Ip.h"

Def::TrialDef::TrialDef( Def *orig ) : orig( orig ) {
}

Def::TrialDef::~TrialDef() {
    delete scope;
}

Var Def::TrialDef::call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var &self, int apply_mode ) {
    if ( apply_mode != Scope::APPLY_MODE_STD )
        TODO;

    // particular case
    if ( orig->name == STRING_init_NUM ) {
        TODO;
        //        TypeInfo *ti = self.type_info();
        //        for( int i = 0; i < ti->attributes.size(); ++i ) {
        //            if ( ti->attributes[ i ].dynamic() ) {
        //                Var subv = scope->get_attr( self, ti->attributes[ i ].name, sf, off );
        //                Var suin = scope->get_attr( subv, STRING_init_NUM, sf, off );
        //                for( int a = 0; ; ++a ) {
        //                    if ( a == orig->attr_init.size() ) {
        //                        if ( ti->attributes[ i ].var.data )
        //                            scope->apply( suin, 1, &ti->attributes[ i ].var, 0, 0, 0, Scope::APPLY_MODE_STD, sf, off );
        //                        else
        //                            scope->apply( suin, 0, 0, 0, 0, 0, Scope::APPLY_MODE_STD, sf, off );
        //                        break;
        //                    }
        //                    AttrInit *ai = &orig->attr_init[ a ];
        //                    if ( ai->name == ti->attributes[ i ].name ) {
        //                        Var vu[ ai->nu ];
        //                        for( int i = 0; i < ai->nu; ++i )
        //                            vu[ i ] = scope->parse( ai->args[ i ].sf, ai->args[ i ].tok );
        //                        Var vn[ ai->nn ];
        //                        for( int i = 0; i < ai->nn; ++i )
        //                            vn[ i ] = scope->parse( ai->args[ i + ai->nu ].sf, ai->args[ i + ai->nu ].tok );
        //                        scope->apply( suin, ai->nu, vu, ai->nn, ai->names.ptr(), vn, Scope::APPLY_MODE_STD, sf, off );
        //                        break;
        //                    }
        //                }
        //            }
        //        }
    }

    // static variables
    String path;
    for( int i = 0; i < orig->arg_names.size(); ++i )
        path += "_" + to_string( *scope->find_var( orig->arg_names[ i ] ).type );
    Scope ns( scope, path );

    // inline call
    return ns.parse( orig->block.sf, orig->block.tok, "calling" );
}

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
    TrialDef *res = new TrialDef( this );

    if ( flags & IR_HAS_COMPUTED_PERT ) return res->wr( "TODO: computed pertinence" );

    // nb arguments
    if ( pnu + pnn + nu + nn < min_nb_args() ) return res->wr( "no enough arguments" );
    if ( pnu + pnn + nu + nn > max_nb_args() ) return res->wr( "To much arguments" );

    res->scope = new Scope( ip->main_scope.ptr(), name + "_" + sf->name + "_" + to_string( off ) );
    if ( self.defined() )
        res->scope->self = self;


    Vec<bool> arg_ok( Size(), arg_names.size(), false );
    if ( pnu + pnn )
        TODO;
    if ( has_varargs() ) {
        Vec<Var> v_args;
        Vec<int> v_names;

        for( int i = 0; i < nn; ++i ) {
            int o = arg_names.first_index_equal_to( names[ i ] );
            if ( o >= 0 ) {
                if ( arg_ok[ o ] )
                    ip->disp_error( "arg is already assigned", true );
                res->scope->reg_var( names[ i ], vn[ i ] );
                arg_ok[ o ] = true;
            } else {
                v_args  << vn   [ i ];
                v_names << names[ i ];
            }
        }

        for( int i = 0; i < arg_names.size(); ++i ) {
            if ( arg_ok[ i ] )
                ip->disp_error( "arg is already assigned", true );
            res->scope->reg_var( arg_names[ i ], vu[ i ] );
            arg_ok[ i ] = true;
        }
        for( int i = arg_names.size(); i < nu; ++i )
            v_args << vu[ i ];


        Var varargs = ip->make_Varargs( v_args, v_names );
        res->scope->reg_var( STRING_varargs_NUM, varargs );
    } else {
        for( int i = 0; i < nn; ++i ) {
            int o = arg_names.first_index_equal_to( names[ i ] );
            if ( o < 0 )
                return res->wr( "name=... does not appear in def args" );
            if ( arg_ok[ o ] )
                ip->disp_error( "arg assigned twice", true );
            res->scope->reg_var( names[ i ], vn[ i ] );
            arg_ok[ o ] = true;
        }
        for( int i = 0; i < nu; ++i ) {
            if ( arg_ok[ i ] )
                ip->disp_error( "arg assigned twice", true );
            res->scope->reg_var( arg_names[ i ], vu[ i ] );
            arg_ok[ i ] = true;
        }
    }

    // default values
    for( int i = 0; i < arg_ok.size(); ++i ) {
        if ( not arg_ok[ i ] ) {
            int j = i - ( arg_names.size() - arg_defaults.size() );
            if ( j < 0 )
                return res->wr( "bad num default arg (weird)" );
            res->scope->reg_var( arg_names[ i ], res->scope->parse( arg_defaults[ j ].sf, arg_defaults[ j ].tok, "making default value" ) );
            arg_ok[ i ] = true;
        }
    }

    // arg constraints
    for( int i = 0; i < arg_constraints.size(); ++i ) {
        Var v = res->scope->find_var( arg_names[ i ] );
        int n = v.type->name;
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
        res->cond = res->scope->parse( condition.sf, condition.tok, "evaluating condition" );
        if ( res->cond.always( false ) )
            return res->wr( "condition = false" );
    } else
        res->cond = Var( &ip->type_Bool, cst( true ) );

    return res;
}
