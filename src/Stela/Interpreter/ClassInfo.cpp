#include "../System/BinStreamReader.h"
#include "../Ir/CallableFlags.h"
#include "../Inst/PointerOn.h"
#include "../Inst/Concat.h"
#include "../Inst/Cst.h"

#include "Interpreter.h"
#include "ClassInfo.h"
#include "TypeInfo.h"
#include "Scope.h"

#include <limits>

ClassInfo::ClassInfo( const Expr &sf, int src_off, BinStreamReader bin, Expr class_ptr ) : CallableInfo_WT( sf, src_off, bin ), class_ptr( class_ptr ), last( 0 ) {
    for( int i = 0, nb_anc = bin.read_positive_integer(); i < nb_anc; ++i )
        ancestors << Code( sf, bin.read_offset() );
}

ClassInfo::~ClassInfo() {
    for( TypeInfo *t = last; t;  ) {
        TypeInfo *l = t;
        t = t->prev;
        delete l;
    }
}

CallableInfo::Trial *ClassInfo::test( int nu, Var *u_args, int nn, int *n_name, Var *v_args, int pnu, Var *pu_args, int pnn, int *pn_name, Var *pn_args, const Expr &sf, int off, Scope *caller ) {
    TrialClass *res = new TrialClass( this );

    if ( flags & IR_HAS_COMPUTED_PERT ) return res->wr( "TODO: computed pertinence" );

    // nb arguments
    if ( pnu + pnn < min_nb_args() ) return res->wr( "no enough arguments" );
    if ( pnu + pnn > max_nb_args() ) return res->wr( "To much arguments" );

    Scope scope = Scope( ip->main_scope, caller );
    res->args.resize( arg_names.size() );

    if ( has_varargs() ) {
        TODO;
    } else {
        for( int i = 0; i < pnu; ++i ) {
            scope.reg_var( arg_names[ i ], pu_args[ i ], sf, off );
            res->args[ i ] = pu_args[ i ];
        }
        bool bad = false;
        for( int i = 0; i < pnn; ++i ) {
            int o = arg_names.first_index_equal_to( pn_name[ i ] );
            if ( o < 0 ) {
                bad = true;
                break;
            }
            if ( res->args[ o ] )
                return res->wr( "arg is already assigned" );
            scope.reg_var( pn_name[ i ], pn_args[ i ], sf, off );
            res->args[ o ] = pn_args[ i ];
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
            Var val = scope.parse( arg_defaults[ j ].sf, arg_defaults[ j ].tok );
            scope.reg_var( arg_names[ i ], val, sf, off );
            res->args[ i ] = val;
        }
    }
    if ( bad )
        return res->wr( "Bad default value index (weird)" );

    // condition
    if ( condition ) {
        Var c = scope.parse( condition.sf, condition.tok );
        TODO;
        //if ( to_bool( c, sf, off ) != true )
        //    return res->wr( "condition = false" );
    }

    return res;
}

// -------------------------------------------------------------------------------------
void ClassInfo::TrialClass::call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const Expr &sf, int off, Scope *caller, Var &res, Expr ext_cond ) {
    // list compatible for the `type_for` procedure
    Var *arg_ptrs[ args.size() ];
    for( int i = 0; i < args.size(); ++i )
        arg_ptrs[ i ] = &args[ i ];

    Var *type = ip->type_for( orig, arg_ptrs );
    TypeInfo *ti = ip->type_info( type->expr() ); ///< parse if necessary

    // start with a unknown cst
    Expr eres;
    if ( ti->static_size_in_bits >= 0 ) {
        eres = cst( 0, 0, ti->static_size_in_bits );
    } else {
        TODO; // eres = undefined cst with unknown size
    }
    caller->set( res, type, eres, sf, off );

    // call init
    // caller->apply( caller->get_attr( res, STRING_init_NUM, sf, off ), nu, vu, nn, names, vn, Scope::APPLY_MODE_STD, sf, off );
}
