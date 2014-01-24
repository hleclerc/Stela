#include "../System/BinStreamReader.h"
#include "../Ir/CallableFlags.h"
#include "../Inst/PointerOn.h"
#include "../Inst/Concat.h"
#include "../Inst/Cst.h"

#include "Interpreter.h"
#include "SourceFile.h"
#include "ClassInfo.h"
#include "TypeInfo.h"
#include "Scope.h"

#include <limits>

ClassInfo::ClassInfo( Interpreter *ip, const PI8 *sf, const PI8 *tok_data, int src_off ) : CallableInfo_WT( sf, tok_data, src_off ) {
    BinStreamReader bin( tok_data + 1 );
    parse_wt( ip, sf, bin );

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

CallableInfo::Trial *ClassInfo::test( int nu, Var *u_args, int nn, int *n_name, Var *v_args, int pnu, Var *pu_args, int pnn, int *pn_name, Var *pn_args, const PI8 *sf, int off, Scope *caller ) {
    Interpreter *ip = caller->interpreter();
    TrialClass *res = new TrialClass;
    res->orig = this;

    if ( flags & IR_HAS_COMPUTED_PERT ) return res->wr( "TODO: computed pertinence" );
    // nb arguments
    if ( pnu + pnn < min_nb_args() ) return res->wr( "no enough arguments" );
    if ( pnu + pnn > max_nb_args() ) return res->wr( "To much arguments" );

    Scope scope = Scope( ip, ip->main_scope, caller );

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

void ClassInfo::TrialClass::call( int nu, Var *vu, int nn, int *names, Var *vn, int pnu, Var *pvu, int pnn, int *pnames, Var *pvn, const PI8 *sf, int off, Var &res, Expr cond, Scope *caller ) {
    Interpreter *ip = caller->interpreter();
    // TypeInfo
    //    Vec<Var>  parameters;
    //    Var       type_var;

    Vec<Var *> arg_ptrs;
    for( int i = 0; i < args.size(); ++i )
        arg_ptrs << &args[ i ];

    Expr cg = concat( /// class expr
        pointer_on( cst( orig->sf, 0, 8 * SourceFile( orig->sf ).tot_size() ), ip->ptr_size() ),
        cst( orig->tok_data - orig->sf ),
        cst( orig->src_off )
    );

    Var *type = ip->type_for( ip->class_info( cg ), arg_ptrs );
    TypeInfo *ti = ip->type_info( type->expr() );
    ASSERT( ti, "not a registered type" );

    PRINT( *type );
    PRINT( ti );
    TODO; // parse type

    res = Var( ip, type, cst( 17 ) );
}
