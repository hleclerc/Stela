#include "../System/BinStreamReader.h"
#include "../Ir/CallableFlags.h"
#include "../Inst/Cst.h"

#include "Interpreter.h"
#include "TypeInfo.h"
#include "DefInfo.h"
#include "Scope.h"

DefInfo::DefInfo( const Expr &sf, int src_off, BinStreamReader bin, Ptr<VarTable> sn ) : CallableInfo_WT( sf, src_off, bin, sn ) {
    if ( flags & IR_HAS_RETURN_TYPE ) {
        if ( name == STRING_init_NUM ) {
            int nb_args = bin.read_positive_integer();
            attr_init.reserve( nb_args );
            for( int n = 0; n < nb_args; ++n ) {
                AttrInit *ai = attr_init.push_back();
                ai->name = ip->glo_nstr( sf, bin.read_positive_integer() );
                ai->nu = bin.read_positive_integer();
                for( int i = 0; i < ai->nu; ++i )
                    ai->args.push_back( sf, bin.read_offset() );
                ai->nn = bin.read_positive_integer();
                for( int i = 0; i < ai->nn; ++i ) {
                    ai->args.push_back( sf, bin.read_offset() );
                    ai->names.push_back( ip->glo_nstr( sf, bin.read_positive_integer() ) );
                }
            }
        } else
            return_type = Code( sf, bin.read_offset() );
    }

    get_of = flags & IR_IS_A_GET ? ip->glo_nstr( sf, bin.read_positive_integer() ) : -1;
    set_of = flags & IR_IS_A_SET ? ip->glo_nstr( sf, bin.read_positive_integer() ) : -1;
    sop_of = flags & IR_IS_A_SOP ? ip->glo_nstr( sf, bin.read_positive_integer() ) : -1;

    //    if ( d->get_of >= 0 or d->set_of >= 0 or d->sop_of >= 0 ) {
    //        SI32 get_of = d->get_of >= 0 ? d->name : -1;
    //        SI32 set_of = d->set_of >= 0 ? d->name : -1;
    //        SI32 sop_of = d->sop_of >= 0 ? d->name : -1;

    //        SI32 orig = std::max( d->get_of, std::max( d->set_of, d->sop_of ) );
    //        if ( Var res = named_vars.get( orig ) ) {
    //            if ( res.type->base_class != &ip->class_GetSetSopDef )
    //                write_error( "There's already a variable named '" + glob_nstr_cor.str( orig ) + "' in current scope (not of type GetSetSopDef)", sf, d->off );
    //            else {
    //                if ( get_of < 0 ) get_of = to_int( res.type->parameters[ 0 ] );
    //                if ( set_of < 0 ) set_of = to_int( res.type->parameters[ 1 ] );
    //                if ( sop_of < 0 ) sop_of = to_int( res.type->parameters[ 2 ] );
    //            }
    //        }

    //        Vec<Var> par;
    //        par << make_int_var( get_of );
    //        par << make_int_var( set_of );
    //        par << make_int_var( sop_of );

    //        Var res;
    //        res.type = ip->class_GetSetSopDef.type_for( this, par, sf, d->off );
    //        res.flags = Var::PARTIAL_INSTANCIATION;

    //        reg_var( orig, res, true, false, sf, d->off );
    //    }
}

CallableInfo::Trial *DefInfo::test( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var &self, const Expr &sf, int off, Scope *caller ) {
    TrialDef *res = new TrialDef( this );

    if ( flags & IR_HAS_COMPUTED_PERT ) return res->wr( "TODO: computed pertinence" );

    // nb arguments
    if ( pnu + pnn + nu + nn < min_nb_args() ) return res->wr( "no enough arguments" );
    if ( pnu + pnn + nu + nn > max_nb_args() ) return res->wr( "To much arguments" );

    res->scope = new Scope( ip->main_scope, caller );
    if ( self )
        res->scope->self = self;


    Vec<bool> arg_ok( Size(), arg_names.size(), false );
    if ( pnu + pnn )
        TODO;
    if ( has_varargs() ) {
        Vec<Var> uv_args, nv_args;
        Vec<int> nv_name;

        for( int i = 0; i < arg_names.size(); ++i ) {
            res->scope->reg_var( arg_names[ i ], vu[ i ], sf, off, false, true );
            arg_ok[ i ] = true;
        }
        for( int i = arg_names.size(); i < nu; ++i )
            uv_args << vu[ i ];

        for( int i = 0; i < nn; ++i ) {
            int o = arg_names.first_index_equal_to( names[ i ] );
            if ( o >= 0 ) {
                if ( arg_ok[ o ] )
                    caller->disp_error( "arg is already assigned", sf, off, true );
                res->scope->reg_var( names[ i ], vn[ i ], sf, off, false, true );
                arg_ok[ o ] = true;
            } else {
                nv_args << vn   [ i ];
                nv_name << names[ i ];
            }
        }

        Var varargs = ip->make_varargs_var( uv_args, nv_args, nv_name );
        res->scope->reg_var( STRING_varargs_NUM, varargs, sf, off, false, true );
    } else {
        for( int i = 0; i < nu; ++i ) {
            res->scope->reg_var( arg_names[ i ], vu[ i ], sf, off, false, true );
            arg_ok[ i ] = true;
        }
        for( int i = 0; i < nn; ++i ) {
            int o = arg_names.first_index_equal_to( names[ i ] );
            if ( o < 0 )
                return res->wr( "name=... does not appear in def" );
            if ( arg_ok[ o ] )
                caller->disp_error( "arg is already assigned", sf, off );
            res->scope->reg_var( names[ i ], vn[ i ], sf, off, false, true );
            arg_ok[ o ] = true;
        }
    }

    // default values
    for( int i = 0; i < arg_ok.size(); ++i ) {
        if ( not arg_ok[ i ] ) {
            int j = i - ( arg_names.size() - arg_defaults.size() );
            if ( j < 0 )
                return res->wr( "bad num default arg (weird)" );
            res->scope->reg_var( arg_names[ i ], res->scope->parse( arg_defaults[ j ].sf, arg_defaults[ j ].tok ), sf, off, false, true );
            arg_ok[ i ] = true;
        }
    }

    // arg constraints
    for( int i = 0; i < arg_constraints.size(); ++i ) {
        Var v = res->scope->find_var( arg_names[ i ] );
        int n = v.class_info()->name;
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
        res->cond = res->scope->parse( condition.sf, condition.tok );
        if ( ip->to_bool( res->cond, condition.sf, condition.tok ) == 0 )
            return res->wr( "condition = false" );
    }

    return res;
}

Ptr<VarTable> DefInfo::static_named_vars_for( const Vec<TypeInfo *> &arg_types ) {
    Ptr<VarTable> &res = static_named_vars[ arg_types ];
    if ( not res )
        res = new VarTable( sn );
    return res;
}

// -------------------------------------------------------------------------------------
DefInfo::TrialDef::TrialDef( DefInfo *orig ) : orig( orig ), scope( 0 ) {
}

DefInfo::TrialDef::~TrialDef() {
    delete scope;
}

Var DefInfo::TrialDef::call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Var &self, const Expr &sf, int off, Scope *caller, int apply_mode ) {
    if ( apply_mode != Scope::APPLY_MODE_STD )
        TODO;

    // particular case
    if ( orig->name == STRING_init_NUM ) {
        TypeInfo *ti = self.type_info();
        for( int i = 0; i < ti->attributes.size(); ++i ) {
            if ( ti->attributes[ i ].dynamic() ) {
                Var subv = scope->get_attr( self, ti->attributes[ i ].name, sf, off );
                Var suin = scope->get_attr( subv, STRING_init_NUM, sf, off );
                for( int a = 0; ; ++a ) {
                    if ( a == orig->attr_init.size() ) {
                        if ( ti->attributes[ i ].var.data )
                            scope->apply( suin, 1, &ti->attributes[ i ].var, 0, 0, 0, Scope::APPLY_MODE_STD, sf, off );
                        else
                            scope->apply( suin, 0, 0, 0, 0, 0, Scope::APPLY_MODE_STD, sf, off );
                        break;
                    }
                    AttrInit *ai = &orig->attr_init[ a ];
                    if ( ai->name == ti->attributes[ i ].name ) {
                        Var vu[ ai->nu ];
                        for( int i = 0; i < ai->nu; ++i )
                            vu[ i ] = scope->parse( ai->args[ i ].sf, ai->args[ i ].tok );
                        Var vn[ ai->nn ];
                        for( int i = 0; i < ai->nn; ++i )
                            vn[ i ] = scope->parse( ai->args[ i + ai->nu ].sf, ai->args[ i + ai->nu ].tok );
                        scope->apply( suin, ai->nu, vu, ai->nn, ai->names.ptr(), vn, Scope::APPLY_MODE_STD, sf, off );
                        break;
                    }
                }
            }
        }
    }

    // static variables
    Vec<TypeInfo *> arg_types;
    for( int i = 0; i < orig->arg_names.size(); ++i )
        arg_types << scope->find_var( orig->arg_names[ i ] ).type_info();
    scope->static_named_vars = orig->static_named_vars_for( arg_types );

    // inline call
    return scope->parse( orig->block.sf, orig->block.tok );
}
