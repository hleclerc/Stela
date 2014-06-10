#include "../Ir/CallableFlags.h"
#include "Class.h"
#include "Type.h"
#include "Cst.h"
#include "Ip.h"

Class::Class() {
}

void Class::write_to_stream( Stream &os ) {
    os << ip->str_cor.str( name );
}

void Class::read_bin( Scope *scope, BinStreamReader &bin ) {
    Callable::read_bin( scope, bin );

    for( int i = 0, nb_anc = bin.read_positive_integer(); i < nb_anc; ++i )
        ancestors << Code( sf, bin.read_offset() );
}

Class::Trial *Class::test( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, Expr self ) {
    TrialClass *res = new TrialClass( this );

    if ( flags & IR_HAS_COMPUTED_PERT ) return res->wr( "TODO: computed pertinence" );

    // nb arguments
    if ( pnu + pnn < min_nb_args() ) return res->wr( "no enough arguments" );
    if ( pnu + pnn > max_nb_args() ) return res->wr( "To much arguments" );

    Scope scope = Scope( &ip->main_scope, 0, "trial_class_" + to_string( name ) );
    res->args.resize( arg_names.size() );

    if ( has_varargs() ) {
        TODO;
    } else {
        for( int i = 0; i < pnu; ++i ) {
            scope.reg_var( arg_names[ i ], pvu[ i ] );
            res->args[ i ] = pvu[ i ];
        }
        bool bad = false;
        for( int i = 0; i < pnn; ++i ) {
            int o = arg_names.first_index_equal_to( pnames[ i ] );
            if ( o < 0 ) {
                bad = true;
                break;
            }
            if ( res->args[ o ] )
                return res->wr( "arg is already assigned" );
            scope.reg_var( pnames[ i ], pvn[ i ] );
            res->args[ o ] = pvn[ i ];
        }
        if ( bad )
            return res->wr( "No argument named as asked" );
    }

    // default values
    bool bad = false;
    for( int i = 0; i < res->args.size(); ++i ) {
        if ( not res->args[ i ] ) {
            int j = i - ( arg_names.size() - arg_defaults.size() );
            if ( j < 0 ) {
                bad = true;
                break;
            }
            Expr val = scope.parse( arg_defaults[ j ].sf, arg_defaults[ j ].tok, "getting default value" );
            scope.reg_var( arg_names[ i ], val );
            res->args[ i ] = val;
        }
    }
    if ( bad )
        return res->wr( "Bad default value index (weird)" );

    // condition
    if ( condition ) {
        res->cond = *scope.parse( condition.sf, condition.tok, "parsing condition" )->get( scope.cond );
        if ( res->cond.always( false ) )
            return res->wr( "condition = false" );
        if ( not res->cond.always( true ) ) {
            ip->disp_error( "class conditions must be known at compile time" );
            return res->wr( "condition not known at compile time" );
        }
    }

    return res;
}

Expr Class::TrialClass::call( int nu, Expr *vu, int nn, int *names, Expr *vn, int pnu, Expr *pvu, int pnn, int *pnames, Expr *pvn, Expr self, int apply_mode ) {
    Type *type = orig->type_for( args );

    // start with a unknown cst
    Expr ret( type );
    if ( apply_mode == Scope::APPLY_MODE_NEW )
        TODO;

    // call init
    if ( apply_mode == Scope::APPLY_MODE_STD )
        ip->main_scope.apply( ip->main_scope.get_attr( ret, STRING_init_NUM ), nu, vu, nn, names, vn, Scope::APPLY_MODE_STD );

    return ret;
}

Expr Class::const_or_copy( Expr &var ) {
    if ( var->type()->orig == ip->class_Callable ) {
        Expr i = ip->main_scope.apply( var, 0, 0, 0, 0, 0, Scope::APPLY_MODE_PARTIAL_INST );
        Expr v = ip->make_type_var( i->type() );
        return const_or_copy( v );
    }
    if ( var.inst->flags & Inst::CONST )
        return var;
    Expr res = ip->main_scope.copy( var );
    res.inst->flags |= Inst::CONST;
    return res;
}

Type *Class::type_for( Vec<Expr> &args ) {
    for( Type *t : types )
        if ( t->parameters == args )
            return t;
    Type *res = new Type( this );
    for( int i = 0; i < args.size(); ++i )
        res->parameters << const_or_copy( args[ i ] );

    types << res;
    return res;
}
