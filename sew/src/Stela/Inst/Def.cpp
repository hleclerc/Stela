#include "../System/UsualStrings.h"
#include "../Ir/CallableFlags.h"
#include "Type.h"
#include "Class.h"
#include "Def.h"
#include "Cst.h"
#include "Ip.h"

Def::TrialDef::TrialDef( Def *orig ) : orig( orig ) {
}

Def::TrialDef::~TrialDef() {
    delete scope;
}

Expr Def::TrialDef::call( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, Expr self, int apply_mode ) {
    if ( apply_mode != Scope::APPLY_MODE_STD )
        TODO;

    // particular case
    if ( orig->name == STRING_init_NUM ) {
        Type *ti = self->type();
        ti->parse();
        for( int i = 0; i < ti->_attributes.size(); ++i ) {
            if ( ti->_attributes[ i ].dyn() ) {
                // ip->push_sf( ti->_attributes[ i ].sf, "init attr" );
                // ip->off = ti->_attributes[ i ].off;

                Expr subv = scope->get_attr( self, ti->_attributes[ i ].name );
                Expr suin = scope->get_attr( subv, STRING_init_NUM );
                for( int a = 0; ; ++a ) {
                    if ( a == orig->attr_init.size() ) {
                        if ( ti->_attributes[ i ].var )
                            scope->apply( suin, 1, &ti->_attributes[ i ].var, 0, 0, 0, Scope::APPLY_MODE_STD );
                        else
                            scope->apply( suin, 0, 0, 0, 0, 0, Scope::APPLY_MODE_STD );
                        break;
                    }
                    AttrInit *ai = &orig->attr_init[ a ];
                    if ( ai->name == ti->_attributes[ i ].name ) {
                        Expr vu[ ai->nu ];
                        for( int i = 0; i < ai->nu; ++i )
                            vu[ i ] = scope->parse( ai->args[ i ].sf, ai->args[ i ].tok, "parsing init value" );
                        Expr vn[ ai->nn ];
                        for( int i = 0; i < ai->nn; ++i )
                            vn[ i ] = scope->parse( ai->args[ i + ai->nu ].sf, ai->args[ i + ai->nu ].tok, "parsing init value" );
                        scope->apply( suin, ai->nu, vu, ai->nn, ai->names.ptr(), vn, Scope::APPLY_MODE_STD );
                        break;
                    }
                }

                // ip->pop_sf();
            }
        }
    }

    // new scope (with local static variables)
    String path = "Def_" + to_string( this );
    TODO;
    //for( int i = 0; i < orig->arg_names.size(); ++i )
    //    path += "_" + to_string( *scope->find_var( orig->arg_names[ i ] ).type );
    Scope ns( scope, 0, path );

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

Callable::Trial *Def::test( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, Expr self ) {
    TrialDef *res = new TrialDef( this );

    if ( flags & IR_HAS_COMPUTED_PERT ) return res->wr( "TODO: computed pertinence" );

    // nb arguments
    if ( pnu + pnn + nu + nn < min_nb_args() ) return res->wr( "no enough arguments" );
    if ( pnu + pnn + nu + nn > max_nb_args() ) return res->wr( "To much arguments" );

    TODO;
    res->scope = new Scope( &ip->main_scope, 0, name + "_" + sf->name + "_" + to_string( off ) );
    if ( self ) {
        res->scope->method = true;
        res->scope->self = self;
    }


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
        //                res->scope->local_vars << vn[ i ];
        //                arg_ok[ o ] = true;
        //            } else {
        //                v_args  << vn   [ i ];
        //                v_names << names[ i ];
        //            }
        //        }

        //        for( int i = 0; i < arg_names.size(); ++i ) {
        //            if ( arg_ok[ i ] )
        //                ip->disp_error( "arg is already assigned", true );
        //            res->scope->local_vars << vu[ i ];
        //            arg_ok[ i ] = true;
        //        }
        //        for( int i = arg_names.size(); i < nu; ++i )
        //            v_args << vu[ i ];


        //        Expr varargs = ip->make_Varargs( v_args, v_names );
        //        res->scope->reg_var( STRING_varargs_NUM, varargs );
    } else {
        // unnamed args
        for( int i = 0; i < nu; ++i )
            res->scope->local_vars << vu[ i ];
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
                    res->scope->local_vars << res->scope->parse( arg_defaults[ j ].sf, arg_defaults[ j ].tok, "making default value" );
                    break;
                }
                if ( arg_name == names[ n ] ) {
                    res->scope->local_vars << vn[ i ];
                    used_arg[ n ] = true;
                    break;
                }
            }
        }
        for( int n = 0; n < nn; ++n ) {
            if ( not used_arg[ n ] ) {
                for( int m = 0; m < n; ++m )
                    if ( names[ n ] == names[ m ] )
                        ip->disp_error( "arg assigned twice", true );
                return res->wr( "name=... does not appear in def args" );
            }
        }
    }

    // arg constraints
    for( int i = 0; i < arg_constraints.size(); ++i ) {
        Expr v = res->scope->local_vars[ i ];
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
        //res->cond = res->scope->parse( condition.sf, condition.tok, "evaluating condition" )->get( res->scope->cond );
        if ( res->cond.always( false ) )
            return res->wr( "condition = false" );
    }

    return res;
}
