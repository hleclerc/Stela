#include "CallableInfo.h"
#include "Interpreter.h"
#include "SourceFile.h"
#include "Scope.h"

#include <algorithm>
#include <limits>

#include "../System/BinStreamWriter.h"

#include "../Inst/PointerOn.h"
#include "../Inst/Syscall.h"
#include "../Inst/Concat.h"
#include "../Inst/Slice.h"
#include "../Inst/ValAt.h"
#include "../Inst/Rand.h"
#include "../Inst/Cst.h"
#include "../Inst/Op.h"

#include "RefExpr.h"
#include "RefPtr.h"

#include "../Ir/AssignFlags.h"
#include "../Ir/Numbers.h"

Scope::Scope( Interpreter *ip, Scope *parent, Scope *caller, Ptr<VarTable> snv ) :
    ip( ip ), parent( parent ), caller( caller ) {

    static_named_vars = snv ? snv : new VarTable;

    do_not_execute_anything = false;
    instantiated_from_sf = 0;
    base_size = 0;
    base_alig = 1;

    sys_state = parent ? parent->sys_state : Var( ip, &ip->type_Void );
    self = 0;
}

Var Scope::parse( const Var *sf, const PI8 *tok ) {
    if ( tok == 0 or do_not_execute_anything )
        return ip->error_var;

    BinStreamReader bin( tok );
    PI8 tva = bin.get<PI8>();
    int off = bin.read_positive_integer();

    switch ( tva ) {
        #define DECL_IR_TOK( N ) case IR_TOK_##N: return parse_##N( sf, off, bin );
        #include "../Ir/Decl.h"
        #undef DECL_IR_TOK
        default: ip->disp_error( "Unknown token type", sf, off, this ); return ip->error_var;
    }
}

Interpreter *Scope::interpreter() {
    return ip;
}

Var Scope::parse_BLOCK( const Var *sf, int off, BinStreamReader bin ) {
    Var res;
    while ( const PI8 *tok = bin.read_offset() )
        res = parse( sf, tok );
    return res;
}

template<class T>
Var Scope::make_var( T val ) {
    Vec<PI8> data( Size(), sizeof( T ) );
    memcpy( data.ptr(), &val, sizeof( T ) );
    return Var( ip, ip->type_for( S<T>() ), cst( data ) );
}

int Scope::read_nstring( const Var *sf, BinStreamReader &bin ) {
    return ip->glo_nstr( sf, bin.read_positive_integer() );
}

Var Scope::copy( const Var &var, const Var *sf, int off ) {
    TODO;
    return var;
}

Var Scope::parse_CALLABLE( const Var *sf, int off, BinStreamReader bin, Var *type ) {
    if ( not sf->cst_data() )
        return disp_error( "TODO: class or def in sourcefile wo cst_data" );
    // we read only the name, because the goal is only to register the def/class
    int bin_offset = bin.ptr - sf->cst_data() - BinStreamWriter::size_needed_for( off ) - 1;
    int name = read_nstring( sf, bin );

    Var res;
    switch( name ) {
    #define DECL_BT( T ) case STRING_##T##_NUM: res = ip->class_##T; break;
    #include "DeclBaseClass.h"
    #include "DeclParmClass.h"
    #undef DECL_BT
    default: res = ::constified( Var( ip, type ) );
    }

    // &sf, bin_offset, off
    res.data->ptr = new RefExpr( concat(
        pointer_on( sf->expr(), ip->ptr_size() ),
        cst( bin_offset ),
        cst( off )
    ) );
    res.flags |= Var::SURDEF;
    reg_var( name, res, sf, off, true, false );

    return ip->void_var;
}

Var Scope::parse_DEF( const Var *sf, int off, BinStreamReader bin ) {
    return parse_CALLABLE( sf, off, bin, &ip->type_Class );
}

Var Scope::parse_CLASS( const Var *sf, int off, BinStreamReader bin ) {
    return parse_CALLABLE( sf, off, bin, &ip->type_Def );
}

Var Scope::parse_RETURN( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_APPLY( const Var *sf, int off, BinStreamReader bin ) {
    Var f = parse( sf, bin.read_offset() );

    int nu = bin.read_positive_integer();
    Var u_args[ nu ];
    for( int i = 0; i < nu; ++i )
        u_args[ i ] = parse( sf, bin.read_offset() );

    int nn = bin.read_positive_integer();
    int n_name[ nn ];
    Var n_args[ nn ];
    for( int i = 0; i < nn; ++i ) {
        n_name[ i ] = read_nstring( sf, bin );
        n_args[ i ] = parse( sf, bin.read_offset() );
    }

    return apply( f, nu, u_args, nn, n_name, n_args, APPLY_MODE_STD, sf, off );
}
Var Scope::parse_SELECT( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_CHBEBA( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_SI32( const Var *sf, int off, BinStreamReader bin ) {
    return make_var( SI32( bin.read_positive_integer() ) );
}
Var Scope::parse_PI32( const Var *sf, int off, BinStreamReader bin ) {
    return make_var( SI32( bin.read_positive_integer() ) );
}
Var Scope::parse_SI64( const Var *sf, int off, BinStreamReader bin ) {
    return make_var( SI64( bin.read_positive_integer() ) );
}
Var Scope::parse_PI64( const Var *sf, int off, BinStreamReader bin ) {
    return make_var( PI64( bin.read_positive_integer() ) );
}
Var Scope::parse_PTR( const Var *sf, int off, BinStreamReader bin ) {
    if ( ip->ptr_size() == 32 )
        return make_var( SI32( bin.read_positive_integer() ) );
    return make_var( SI64( bin.read_positive_integer() ) );
}

Var Scope::parse_STRING( const Var *sf, int off, BinStreamReader bin ) {
    TODO; return Var();
}

Var Scope::parse_VAR( const Var *sf, int off, BinStreamReader bin ) {
    int name = read_nstring( sf, bin );
    if ( Var res = find_var( name ) )
        return res;
    return disp_error( "Impossible to find variable '" + glob_nstr_cor.str( name ) + "'.", sf, off );
}

Var Scope::parse_ASSIGN( const Var *sf, int off, BinStreamReader bin ) {
    // name
    int name = read_nstring( sf, bin );

    // flags
    int flags = bin.read_positive_integer();

    // inst
    Var var = parse( sf, bin.read_offset() );

    //
    if ( flags & IR_ASSIGN_TYPE ) {
        // var = apply( var, 0, 0, 0, 0, 0, true, class_scope ? APPLY_MODE_PARTIAL_INST : APPLY_MODE_STD, sf, off );
        TODO;
    }

    if ( ( flags & IR_ASSIGN_REF ) == 0 and var.referenced_more_than_one_time() )
        var = copy( var, sf, off );

    if ( flags & IR_ASSIGN_CONST ) {
        if ( ( var.flags & Var::WEAK_CONST ) == 0 and var.referenced_more_than_one_time() )
            disp_error( "Impossible to make this variable fully const (already referenced elsewhere)", sf, off );
        var.flags |= Var::WEAK_CONST;
        var.data->flags = PRef::CONST;
    }

    return reg_var( name, var, sf, off, flags & IR_ASSIGN_STATIC );
}

Var Scope::parse_REASSIGN( const Var *sf, int off, BinStreamReader bin ) {
    TODO; return Var();
}

Var Scope::get_attr( Var self, int attr ) {
    if ( ip->isa_Error( self ) )
        return self;

    PRINT( self );
    TODO; return Var();

    //    if ( self.type->base_class == &ip->class_GetSetSopInst )
    //        return get_attr( get_val_GetSetSopInst( self, sf, off ), name, sf, off );

    //    // in self or super(s)
    //    Var res = get_attr_rec( self, name );

    //    // look for external methods
    //    if ( not res ) {
    //        for( Scope *s = this; s; s = s->parent ) {
    //            if ( s->stat_named_vars and ( res = ext_method( s->stat_named_vars->get( name ) ) ) )
    //                break;
    //            if ( res = ext_method( s->named_vars.get( name ) ) )
    //                break;
    //        }
    //    }

    //    if ( res ) {
    //        if ( res.type->base_class == &ip->class_GetSetSopDef ) {
    //            SI32 get_of = to_int( res.type->parameters[ 0 ] );
    //            Var getr;
    //            if ( get_of >= 0 )
    //                getr = apply( get_attr( self, get_of, sf, off ), 0, 0, 0, 0, 0, true );

    //            Vec<Var> par;
    //            par << make_type_var( self.type );
    //            par << make_type_var( getr.type );
    //            for( int i = 0; i < 3; ++i )
    //                par << res.type->parameters[ i ];
    //            Var tmp( ip, ip->class_GetSetSopInst.type_for( this, par, sf, off ) );
    //            ip->set_ref( tmp.data + 0 * sizeof( void * ), self );
    //            ip->set_ref( tmp.data + 1 * sizeof( void * ), getr );
    //            return tmp;
    //        }


    //        if ( res.flags & Var::CALLABLE ) {
    //            // find all the variants
    //            Vec<Var> lst;

    //            // in attributes
    //            if ( const Type::Attr *attr = self.type->find_attr( name ) )
    //                lst << self.type->make_attr( self, attr );

    //            // static scope
    //            if ( self.type->stat_named_vars )
    //                self.type->stat_named_vars->get( lst, name );
    //            if ( name != STRING_init_NUM and name != STRING_destroy_NUM )
    //                for( int i = 0; i < self.type->ancestors.size(); ++i )
    //                    if ( self.type->ancestors[ i ]->stat_named_vars )
    //                        self.type->ancestors[ i ]->stat_named_vars->get( lst, name );


    //            // external methods
    //            int os = lst.size();
    //            for( Scope *s = this; s; s = s->parent ) {
    //                if ( s->stat_named_vars )
    //                    s->stat_named_vars->get( lst, name );
    //                s->named_vars.get( lst, name );
    //            }
    //            for( int i = os; i < lst.size(); ++i )
    //                if ( not ext_method( lst[ i ] ) )
    //                    lst.remove_unordered( i-- );

    //            // template parameters of SurdefList
    //            Vec<Var> par;
    //            par << make_type_var( self.type );
    //            par << make_surdefs_var( lst );
    //            par << make_type_var( make_varargs_var( Vec<Var>() ).type ); // parameters
    //            Var res( ip, ip->class_SurdefList.type_for( this, par, sf, off ) );
    //            ip->set_ref( res.data + 0 * sizeof( void * ), self  );
    //            ip->set_ref( res.data + 1 * sizeof( void * ), Var() );
    //            return res;
    //        }

    //        return res;
    //    }

    //    write_error( "no attr '" + glob_nstr_cor.str( name ) + "' in object of type '" + to_string( *self.type ) + "'", sf, off );
    //    return ip->error_var;
}

Var Scope::parse_GET_ATTR( const Var *sf, int off, BinStreamReader bin ) {
    Var self = parse( sf, bin.read_offset() );
    int attr = read_nstring( sf, bin );
    if ( Var res = get_attr( self, attr ) )
        return res;
    return disp_error( "No attribute " + glob_nstr_cor.str( attr ), sf, off );

}
Var Scope::parse_GET_ATTR_PTR( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_GET_ATTR_ASK( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_GET_ATTR_PTR_ASK( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_GET_ATTR_PA( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_IF( const Var *sf, int off, BinStreamReader bin ) {
    Var cond = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );

    // bool conversion
    if ( not ip->isa_Bool( cond ) ) {
        // cond = apply();
    }

    // simplified expression
    Expr expr = simplified_expr( cond );
    PRINT( expr );

    // known value
    //    if ( const PI8 *data = expr.cst_data() ) {
    //        if ( cond.type == &ip->type_Bool or cond.type == &ip->type_Bool or  ) {
    //        }
    //    }

    TODO;
    return ip->error_var;

}
Var Scope::parse_WHILE( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_BREAK( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_CONTINUE( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_FALSE( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_TRUE( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_VOID( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_SELF( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_THIS( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_FOR( const Var *sf, int off, BinStreamReader bin ) {
    //     int nn = bin.read_positive_integer();
    //     SI32 names[ nn ];
    //     for( int i = 0; i < nn; ++i )
    //         names[ i ] = read_nstring( sf, bin );
    //     int nc = bin.read_positive_integer();
    //     Var objects[ nc ];
    //     for( int i = 0; i < nc; ++i ) {
    //         objects[ i ] = parse( sf, bin.read_offset() );
    //         if ( objects[ i ].type == ip->type_Error )
    //             return objects[ i ];
    //     }
    //     const PI8 *code = bin.read_offset();
    //     int off = bin.read_positive_integer();
    //
    //     if ( nn != 1 or nc != 1 )
    //         TODO;
    //
    //     //
    //     Vec<Var> var_names;
    //     for( int i = 0; i < nn; ++i )
    //         var_names << make_int_var( names[ i ] );
    //
    //     // make a block var
    //     Vec<Var> block_par;
    //     block_par << make_int_var( ST( sf ) );
    //     block_par << make_int_var( ST( code ) );
    //     block_par << make_varargs_var( var_names );
    //     Type *block_type = ip->class_Block.type_for( this, block_par );
    //     Var block( ip, block_type );
    //     *reinterpret_cast<Scope **>( block.data ) = this;
    //
    //     Var f = get_attr( objects[ 0 ], STRING___for___NUM, sf, off );
    //     apply( f, 1, &block, 0, 0, 0, false, APPLY_MODE_STD, sf, off );
    //     return ip->void_var;
    TODO; return ip->void_var;
}
Var Scope::parse_IMPORT( const Var *sf, int off, BinStreamReader bin ) {
    //     ST size = bin.read_positive_integer();
    //     char data[ size + 1 ];
    //     bin.read( data, size );
    //     data[ size ] = 0;
    //     int offset = bin.read_positive_integer();
    //     import( data, sf, offset );
    //     return ip->void_var;
    TODO; return ip->void_var;
}
Var Scope::parse_NEW( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
Var Scope::parse_LIST( const Var *sf, int off, BinStreamReader bin ) {
    //     int dim = bin.read_positive_integer();
    //
    //     Vec<Var> data;
    //     Vec<int> size;
    //     parse_LIST_rec( data, size, sf, bin, dim );
    //
    //     Vec<Var> parameters;
    //     parameters << make_int_var( dim );
    //     parameters << Var( ip, make_RefArray_size_type( size ) );
    //     parameters << make_type_var( make_RefArray_data_type( data ) );
    //     Var res( ip, ip->class_RefArray.type_for( this, parameters ) );
    //     for( int i = 0; i < data.size(); ++i )
    //         ip->set_ref( res.data + i * sizeof( void * ), data[ i ] );
    //
    //     return res;
    TODO; return ip->void_var;
}
Var Scope::parse_LAMBDA( const Var *sf, int off, BinStreamReader bin ) {
    //     int nb_vars = bin.read_positive_integer();
    //     Vec<SI32> strs;
    //     for( int i = 0; i < nb_vars; ++i )
    //         strs << read_nstring( sf, bin );
    //
    //     Vec<Var> vars;
    //     for( int i = 0; i < nb_vars; ++i )
    //         vars << make_int_var( strs[ i ] );
    //
    //     const PI8 *tk = bin.read_offset();
    //
    //     FindVariables fv;
    //     fv.scope = this;
    //     fv.pre = strs;
    //     fv.sf = sf;
    //     fv.parse( tk );
    //
    //     Vec<Var> lst_n;
    //     Vec<int> names;
    //     for( int i = 0; i < fv.var.size(); ++i ) {
    //         Var res = find_var( fv.var[ i ], main_scope() );
    //         if ( res ) {
    //             names << fv.var[ i ];
    //             lst_n << res;
    //         }
    //     }
    //     Var ext_refs = make_varargs_var( Vec<Var>(), lst_n, names );
    //
    //     Vec<Var> parameters;
    //     parameters << make_varargs_var( vars );
    //     parameters << make_int_var( (SI64)sf );
    //     parameters << make_int_var( (SI64)tk );
    //     parameters << make_type_var( ext_refs.type );
    //     Var res( ip, ip->class_LambdaFunc.type_for( this, parameters ) );
    //     memcpy( res.data, ext_refs.data, ext_refs.type->size_in_bytes() );
    //     for( int i = 0; i < lst_n.size(); ++i )
    //         ip->set_ref( res.data + i * sizeof( void * ), lst_n[ i ] );
    //     return res;
    TODO; return ip->void_var;
}
Var Scope::parse_NULL_REF( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }

Var Scope::get_val_if_GetSetSopInst( const Var &val ) {
    return val;
}

ErrorList::Error &Scope::make_error( String msg, const Var *sf, int off, bool warn ) {
    return ip->make_error( msg, sf, off, this, warn );
}

Var Scope::disp_error( String msg, const Var *sf, int off, bool warn ) {
    ip->disp_error( msg, sf, off, this, warn );
    return ip->error_var;
}

Expr Scope::simplified_expr( const Var &var ) {
    Var sop = get_val_if_GetSetSopInst( var );
    return sop.expr();
}

struct CmpCallableInfobyPertinence {
    bool operator()( const CallableInfo *a, const CallableInfo *b ) const {
        return a->pertinence > b->pertinence;
    }
};

Var Scope::apply( const Var &f, int nu, Var *u_args, int nn, int *n_names, Var *n_args, ApplyMode am, const Var *sf, int off ) {
    // if error_var -> break
    if ( ip->isa_Error( f ) )
        return ip->error_var;
    for( int i = 0; i < nu; ++i )
        if ( ip->isa_Error( u_args[ i ] ) )
            return ip->error_var;
    for( int i = 0; i < nn; ++i )
        if ( ip->isa_Error( n_args[ i ] ) )
            return ip->error_var;

    //int na = nu + nn;
    if ( ip->isa_Callable( f ) ) {
        // Callable[ surdef_list, self_type, parm_type ]
        //  - self ref
        //  - parm data
        // type = class ptr + parameter refs
        SI32 nb_surdefs = 0, ps = ip->ptr_size();
        Expr surdef_list_ptr = slice( f.type->ptr->expr(), 1 * ps, 2 * ps );
        if ( not val_at( surdef_list_ptr, 32 ).basic_conv( nb_surdefs ) )
            return disp_error( "pb decoding callable (to find surdef list)" );
        Expr surdef_list = val_at( surdef_list_ptr, 32 + nb_surdefs * ps );

        int pnu = 0, pnn = 0, *pn_names = 0;
        Var *pu_args = 0, *pn_args = 0;

        CallableInfo *ci[ nb_surdefs ];
        for( int i = 0; i < nb_surdefs; ++i ) {
            Expr surdef = val_at( slice( surdef_list, 32 + ( i + 0 ) * ps, 32 + ( i + 1 ) * ps ), ip->ptr_size() + 64 );
            ci[ i ] = ip->callable_info( surdef );
        }
        std::sort( ci, ci + nb_surdefs, CmpCallableInfobyPertinence() );

        int nb_ok = 0;
        double guaranted_pertinence = 0;
        bool   has_guaranted_pertinence = false;
        AutoPtr<CallableInfo::Trial> trials[ nb_surdefs ];
        for( int i = 0; i < nb_surdefs; ++i ) {
            if ( has_guaranted_pertinence and guaranted_pertinence > ci[ i ]->pertinence ) {
                trials[ i ] = new CallableInfo::Trial( "Already a more pertinent solution" );
                break;
            }

            trials[ i ] = ci[ i ]->test( nu, u_args, nn, n_names, n_args, pnu, pu_args, pnn, pn_names, pn_args, sf, off, this );

            if ( trials[ i ]->ok() ) {
                if ( not trials[ i ]->cond ) {
                    has_guaranted_pertinence = true;
                    guaranted_pertinence = ci[ i ]->pertinence;
                }
                ++nb_ok;
            }
        }

        //
        for( int i = 0; i < nb_surdefs; ++i )
            if ( ip->isa_Error( trials[ i ]->cond ) )
                return ip->error_var;

        // no valid surdef ?
        if ( nb_ok == 0 ) {
            std::ostringstream ss;
            ss << "No matching surdef";
            //            if ( self ) {
            //                ss << " (looking for '" << *self.type;
            //                if ( lst_def.size() )
            //                    ss << "." << glob_nstr_cor.str( lst_def[ 0 ]->name );
            //                ss << "' with " << na << " argument";
            //                ss << ")";
            //            }
            ErrorList::Error &err = make_error( ss.str(), sf, off );
            for( int i = 0; i < nb_surdefs; ++i )
                err.ap( ci[ i ]->filename(), ci[ i ]->off(), std::string( "possibility (" ) + trials[ i ]->reason + ")" );
            std::cerr << err;
            return ip->error_var;
        }

        // valid surdefs, but only with runtime conditions
        if ( not has_guaranted_pertinence ) {
            std::ostringstream ss;
            ss << "There is no failback surdefinition (only runtime conditions)";
            ErrorList::Error &err = make_error( ss.str(), sf, off );
            for( int i = 0; i < nb_surdefs; ++i ) {
                if ( trials[ i ]->ok() )
                    err.ap( ci[ i ]->filename(), ci[ i ]->off(), "accepted" );
                else
                    err.ap( ci[ i ]->filename(), ci[ i ]->off(), std::string( "rejected (" ) + trials[ i ]->reason + ")" );
            }
            std::cerr << err;
            return ip->error_var;
        }

        // ambiguous overload ?
        if ( nb_ok > 1 ) {
            double best_pertinence = -std::numeric_limits<double>::max();
            for( int i = 0; i < nb_surdefs; ++i )
                best_pertinence = std::max( best_pertinence, ci[ i ]->pertinence );
            int nb_wp = 0;
            for( int i = 0; i < nb_surdefs; ++i )
                nb_wp += ci[ i ]->pertinence == best_pertinence;
            if ( nb_wp > 1 ) {
                std::ostringstream ss;
                ss << "Ambiguous overload";
                ErrorList::Error &err = make_error( ss.str(), sf, off );
                for( int i = 0; i < nb_surdefs; ++i ) {
                    if ( trials[ i ]->ok() and ci[ i ]->pertinence == best_pertinence )
                        err.ap( ci[ i ]->filename(), ci[ i ]->off(), "possibility" );
                }
                std::cerr << err;
                return ip->error_var;
            }
        }

        Var res;
        Expr cond = cst( true );
        for( int i = 0; i < nb_surdefs; ++i ) {
            if ( trials[ i ]->ok() ) {
                Expr ok_cond;
                if ( trials[ i ]->cond )
                    ok_cond = and_op( bt_Bool, cond, trials[ i ]->cond.expr() );
                else
                    ok_cond = cond;

                trials[ i ]->call( nu, u_args, nn, n_names, n_args, pnu, pu_args, pnn, pn_names, pn_args, sf, off, res, ok_cond, this );

                if ( trials[ i ]->cond )
                    cond = and_op( bt_Bool, cond, not_op( bt_Bool, trials[ i ]->cond.expr() ) );
                else
                    break;
            }
        }

        return res;
    }

    disp_error( "unmanaged callable type", sf, off );
    PRINT( f );
    TODO;
    return ip->error_var;
}

void Scope::set( Var &o, Expr n, const Var *sf, int off ) {
    if ( not o.set( n ) )
        disp_error( "const slot", sf, off );
}

Var Scope::reg_var( int name, const Var &var, const Var *sf, int off, bool stat, bool check ) {
    if ( check and ( named_vars.get( name ) or static_named_vars->get( name ) ) )
        return disp_error( "There is already a variable named '" + glob_nstr_cor.str( name ) + "' in the current scope", sf, off );
    VarTable *vt = stat ? static_named_vars.ptr() : &named_vars;
    vt->reg( name, var );
    return var;
}

Var Scope::find_var_first( int name ) {
    for( Scope *s = this; ; s = s->parent ) {
        if ( Var res = s->named_vars.get( name ) )
            return res;
        if ( Var res = s->static_named_vars->get( name ) )
            return res;
        //
        if ( Scope *p = s->parent ) {
            if ( not p->parent ) { // -> there is a parent, but the parent is the main scope
                for( VarTable *vt = p->static_named_vars.ptr(); vt; vt = vt->parent )
                    if ( Var res = vt->get( name ) )
                        return res;
                // non static vars of main scope
                if ( Var res = p->named_vars.get( name ) )
                    return res;
                return Var();
            }
        } else
            break;
    }
    return Var();
}

void Scope::find_var_clist( Vec<Var> &lst, int name ) {
    for( Scope *s = this; ; s = s->parent ) {
        s->named_vars.get( lst, name );
        s->static_named_vars->get( lst, name );
        //
        if ( Scope *p = s->parent ) {
            if ( not p->parent ) { // -> there is a parent, but the parent is the main scope
                for( VarTable *vt = p->static_named_vars.ptr(); vt; vt = vt->parent )
                    vt->get( lst, name );
                // non static vars of main scope
                p->named_vars.get( lst, name );
            }
        } else
            break;
    }
}

Var Scope::find_var( int name ) {
    Var res = find_var_first( name );
    if ( not res.is_surdef() )
        return res;

    // -> Callable[ surdef_list, self_type, parm_type ]
    Vec<Var> lst;
    find_var_clist( lst, name );
    Expr surdef_list_data = cst( SI32( lst.size() ) );
    for( int i = 0; i < lst.size(); ++i )
        surdef_list_data = concat( surdef_list_data, pointer_on( lst[ i ].expr(), ip->ptr_size() ) );
    Var surdef_list( ip, &ip->type_SurdefList, surdef_list_data );
    Var self_type = self_var() ? ip->type_of( *self_var() ) : ip->void_var;

    Vec<Var *> parms;
    parms << &surdef_list;
    parms << &self_type;
    parms << &ip->void_var;
    Var *callable_type = ip->type_for( ip->class_info( ip->class_Callable ), parms );

    Expr callable_data;
    if ( Var *s = self_var() )
        callable_data = ip->ref_expr_on( *s );
    else
        callable_data = cst();
    return Var( ip, callable_type, callable_data );
}

Var *Scope::self_var() {
    if ( self )
        return self;
    return parent ? parent->self_var() : 0;
}

#define CHECK_PRIM_ARGS( N ) \
    int n = bin.read_positive_integer(); \
    if ( n != N ) \
        return disp_error( "Expecting " #N " operand", sf, off );

Var Scope::parse_rand( const Var *sf, int off, BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 0 );
    return Var( ip, &ip->type_PI64, rand( ip->ptr_size() ) );
}

Var Scope::parse_syscall( const Var *sf, int off, BinStreamReader bin ) {
    int n = bin.read_positive_integer();
    Expr inp[ n ];
    for( int i = 0; i < n; ++i ) {
        Var v = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
        if ( not ip->isa_ptr_int( v ) )
            return disp_error( "Expecting size types (size of a pointer)", sf, off );
        inp[ i ] = simplified_expr( v );
    }

    syscall res( simplified_expr( sys_state ), n, inp, ip->ptr_size() );
    set( sys_state, res.sys, sf, off );
    return Var( ip, &ip->type_SI64, res.ret );
}

Var Scope::parse_typeof( const Var *sf, int off, BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 1 );
    Var a = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
    return ip->type_of( a );
}

Var Scope::parse_set_base_size_and_alig( const Var *sf, int off, BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 2 );
    Var a = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
    Var b = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
    if ( ip->isa_Error( a ) or ip->isa_Error( b ) )
        return ip->void_var;
    if ( not ip->conv( base_size, a ) or not ip->conv( base_alig, b ) )
        return disp_error( "set_base_size_and_alig -> SI32/SI64 types only", sf, off );
    return ip->void_var;
}

Var Scope::parse_size_of( const Var *sf, int off, BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 1 );
//     Var a = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     Var r = apply( a, 0, 0, 0, 0, 0, true, APPLY_MODE_PARTIAL_INST, sf, bin.read_positive_integer() ); // partial_instanciation
// 
//     Var res( ip, ip->type_for( S<ST>() ) );
//     *reinterpret_cast<ST *>( res.data ) = r.type->size_in_bits;
//     return res;
    TODO; return ip->void_var;
}

Var Scope::parse_alig_of( const Var *sf, int off, BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 1 );
//     Var a = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     Var r = apply( a, 0, 0, 0, 0, 0, true, APPLY_MODE_PARTIAL_INST, sf, bin.read_positive_integer() ); // partial_instanciation
// 
//     Var res( ip, ip->type_for( S<ST>() ) );
//     *reinterpret_cast<ST *>( res.data ) = r.type->alig_in_bits;
//     return res;
    TODO; return ip->void_var;
}

Var Scope::parse_select_SurdefList( const Var *sf, int off, BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 2 );
//     Var surdef_list = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     Var varargs = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
// 
//     if ( surdef_list.type == ip->type_Class ) {
//         Vec<Var> lst;
//         lst << surdef_list;
// 
//         Vec<Var> par;
//         par << ip->void_var;
//         par << make_surdefs_var( lst );
//         par << make_type_var( varargs.type );
//         Var res( ip, ip->class_SurdefList.type_for( this, par ) );
//         ip->set_ref( res.data + 0 * sizeof( void * ), Var() );
//         ip->set_ref( res.data + 1 * sizeof( void * ), varargs );
//         return res;
//     }
// 
//     // -> SurdefList
//     Vec<Var> par;
//     par << surdef_list.type->parameters[ 0 ];
//     par << surdef_list.type->parameters[ 1 ];
//     par << make_type_var( varargs.type );
//     Var res( ip, ip->class_SurdefList.type_for( this, par ) );
//     ip->set_ref( res.data + 0 * sizeof( void * ), surdef_list );
//     ip->set_ref( res.data + 1 * sizeof( void * ), varargs );
//     return res;
    TODO; return ip->void_var;
}

Var Scope::parse_address( const Var *sf, int off, BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 1 );
//     Var a = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
// 
//     Var res( ip, ip->type_for( S<ST>() ) );
//     *reinterpret_cast<ST *>( res.data ) = ST( a.data );
//     return res;
    TODO; return ip->void_var;
}

Var Scope::parse_get_argc( const Var *sf, int off, BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 0 );
//     return make_int_var( ip->argc );
    TODO; return ip->void_var;
}

Var Scope::parse_get_argv( const Var *sf, int off, BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 1 );
//     Var a = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     int num = to_int( a );
// 
//     if ( num >= ip->argc )
//         return ip->error_var;
//     return make_PermanentString( ip->argv[ num ], strlen( ip->argv[ num ] ) );
    TODO; return ip->void_var;
}

Var Scope::parse_apply_LambdaFunc( const Var *sf, int off, BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 2 );
//     Var lfun = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     Var args = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     int off = bin.read_positive_integer();
// 
//     // new Scope
//     Scope scope( ip, main_scope(), this );
//     Var names = lfun.type->parameters[ 0 ];
//     for( unsigned i = 0; i < names.type->size_in_bytes() / sizeof( SI32 * ); ++i ) {
//         Var num = make_int_var( SI32( i ) );
//         Var val = apply( get_attr( args, STRING_select_NUM, sf, off ), 1, &num, 0, 0, 0, true );
//         scope.reg_var( *reinterpret_cast<SI32 **>( names.data )[ i ], val, false );
//     }
// 
//     Var ext = get_attr( lfun, STRING_ext_refs_NUM, sf, off );
//     Var nb_ext = apply( get_attr( ext, STRING_get_size_NUM, sf, off ), 0, 0, 0, 0, 0, true );
//     for( int i = 0; i < nb_ext; ++i ) {
//         Var num = make_int_var( SI32( i ) );
//         Var str = apply( get_attr( ext, STRING_name_NUM  , sf, off ), 1, &num, 0, 0, 0, true );
//         Var val = apply( get_attr( ext, STRING_select_NUM, sf, off ), 1, &num, 0, 0, 0, true );
//         scope.reg_var( to_int( str ), val );
//     }
// 
//     return scope.parse(
//         reinterpret_cast<const SourceFile *>( to_int( lfun.type->parameters[ 1 ] ) ),
//         reinterpret_cast<const PI8        *>( to_int( lfun.type->parameters[ 2 ] ) )
//     );
    TODO; return ip->void_var;
}

// bool Scope::_inst_of( const Type *inst, const Var &type, const SourceFile *sf, int off, bool strict ) {
//     if ( type.type->base_class == &ip->class_Type ) {
//         return inst->is_extended_from( type_of_type_var( type ), strict );
//     }
//     if ( type.type == ip->type_Class ) {
//         return inst->is_extended_from( reinterpret_cast<Class *>( type.data ), strict );
//     }
//     if ( type.type->base_class == &ip->class_SurdefList ) {
//         Var a = apply( type, 0, 0, 0, 0, 0, true, APPLY_MODE_PARTIAL_INST, sf, off );
//         return inst->is_extended_from( a.type, strict );
//     }
//     PRINT( *type.type );
//     TODO;
//     return 0;
// }

Var Scope::parse_inst_of( const Var *sf, int off, BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 2 );
//     Var inst = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     Var type = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     int off = bin.read_positive_integer();
//     return make_bool_var( _inst_of( inst.type, type, sf, off, false ) );
    TODO; return ip->void_var;
}


Var Scope::parse_pointed_value( const Var *sf, int off, BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 2 );
//     Var t = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     Var a = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     int off = bin.read_positive_integer();
// 
//     if ( a.type == ip->type_RawRef ) {
//         Var res = ip->get_ref( *reinterpret_cast<PI8 **>( a.data ) );
//         if ( not res )
//             return write_error( "not a registered ref", sf, off );
//         res.type = type_of_type_var( t );
//         return res;
//     }
// 
//     Var res( ip, type_of_type_var( t ) );
//     res.data  = *reinterpret_cast<PI8 **>( a.data );
//     res.flags = Var::IS_A_REF;
//     return res;
    TODO; return ip->void_var;
}

Var Scope::parse_set_RawRef_dependancy( const Var *sf, int off, BinStreamReader bin ) {
    TODO;
//     CHECK_PRIM_ARGS( 2 );
//     Var ref = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     Var var = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     PI8 *&ptr = *reinterpret_cast<PI8 **>( ref.data );
// 
//     // if there's an already associated ref
//     ip->raw_refs.erase( ptr );
// 
//     // store a reference on var and save it to ptr
//     ip->raw_refs[ var.data ] = var;
//     ptr = var.data;
    return ip->void_var;
}


// void Scope::parse_LIST_rec( Vec<Var> &data, Vec<int> &size, const SourceFile *sf, BinStreamReader &bin, int nb_dim ) {
//     ST nb_val = bin.read_positive_integer();
//     size << nb_val;
// 
//     if ( nb_dim == 1 )
//         for( int i = 0; i < nb_val; ++i )
//             data << parse( sf, bin.read_offset() );
//     else
//         for( int i = 0; i < nb_val; ++i )
//             parse_LIST_rec( data, size, sf, bin, nb_dim - 1 );
// }


// struct FindVariables : RecIrParser {
//     virtual void parse_VAR( BinStreamReader bin ) {
//         int ns = scope->read_nstring( sf, bin );
//         if ( not pre.contains( ns ) and not var.contains( ns ) )
//             var << ns;
//     }
//     const SourceFile *sf;
//     Vec<SI32> pre;
//     Scope *scope;
//     Vec<int> var;
// };


// Type *Scope::make_RefArray_size_type( const Vec<int> &values, int o ) {
//     if ( o < values.size() ) {
//         Vec<Var> parameters;
//         parameters << make_int_var( values[ o ] );
//         parameters << make_type_var( make_RefArray_size_type( values, o + 1 ) );
//         return ip->class_BegItemSizeRefArray.type_for( this, parameters );
//     }
//     Vec<Var> parameters;
//     return ip->class_EndItemSizeRefArray.type_for( this, parameters );
// }
// 
// Type *Scope::make_RefArray_data_type( const Vec<Var> &values, int o ) {
//     if ( o < values.size() ) {
//         Vec<Var> parameters;
//         parameters << make_type_var( values[ o ].type );
//         parameters << make_type_var( make_RefArray_data_type( values, o + 1 ) );
//         return ip->class_BegItemDataRefArray.type_for( this, parameters );
//     }
//     Vec<Var> parameters;
//     return ip->class_EndItemDataRefArray.type_for( this, parameters );
// }

Var Scope::parse_reassign_rec( const Var *sf, int off, BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 2 );
//     Var a = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     Var b = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     int off = bin.read_positive_integer();
//     if ( a.type == ip->type_Error )
//         return a;
//     if ( b.type == ip->type_Error )
//         return b;
//     if ( a.type != b.type ) {
//         std::ostringstream ss;
//         ss << "not the same types... do not known how to reassign (" << *a.type << " and " << *b.type << ")";
//         return write_error( ss.str(), sf, off );
//     }
//     if ( a.type == ip->type_RawRef ) {
//         if ( a.data )
//             TODO;
//         ip->set_ref( a.data, ip->get_ref( *reinterpret_cast<PI8 **>( b.data ) ) );
//     }
//     for( int i = 0; i < a.type->attributes.size(); ++i ) {
//         if ( a.type->attributes[ i ].offset >= 0 ) {
//             Var sa = a.type->make_attr( a, &a.type->attributes[ i ] );
//             Var sb = b.type->make_attr( b, &b.type->attributes[ i ] );
//             apply( get_attr( sa, STRING_reassign_NUM, sf, off ), 1, &sb, 0, 0, 0, false );
//         }
//     }
//     return ip->void_var;
    TODO; return ip->void_var;
}

Var Scope::parse_set_ptr_val( const Var *sf, int off, BinStreamReader bin ) {
//     CHECK_PRIM_ARGS( 2 );
//     write_error( "...", sf, bin.read_positive_integer() );
//     TODO;
//     Var ptr = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     Var val = get_val_if_GetSetSopInst( parse( sf, bin.read_offset() ) );
//     if ( ptr.type != ip->type_RawPtr )
//         return write_error( "expecting a raw ptr" );
// 
//     *reinterpret_cast<ST *>( ptr.data ) = to_int( val );
    TODO; return ip->void_var;
}

Var Scope::parse_block_exec( const Var *sf, int off, BinStreamReader bin ) {
//     CHECK_PRIM_ARGS( 5 );
//     const SourceFile *sf_ptr = rcast( to_int( parse( sf, bin.read_offset() ) ) );
//     const PI8        *tk_ptr = rcast( to_int( parse( sf, bin.read_offset() ) ) );
//     Scope            *sc_ptr = rcast( to_int( parse( sf, bin.read_offset() ) ) );
//     Var v_names = parse( sf, bin.read_offset() );
//     Var val     = parse( sf, bin.read_offset() );
//     int off     = bin.read_positive_integer();
// 
//     int nn = v_names.type->size_in_bytes() / sizeof( SI32 * );
// 
//     Scope scope( ip, sc_ptr, this );
//     if ( nn != 1 )
//         TODO;
//     for( int i = 0; i < nn; ++i )
//          scope.reg_var( *reinterpret_cast<SI32 **>( v_names.data )[ i ], val, false, true, sf, off );
//     scope.parse( sf_ptr, tk_ptr );
// 
//     return ip->void_var;
    TODO; return ip->void_var;
}

Var Scope::parse_ptr_size( const Var *sf, int off, BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 0 );
    if ( ip->ptr_size() == 32 )
        return make_var( SI32( ip->ptr_size() ) );
    return make_var( SI64( ip->ptr_size() ) );
}

Var Scope::parse_ptr_alig( const Var *sf, int off, BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 0 );
    if ( ip->ptr_size() == 32 )
        return make_var( SI32( ip->ptr_alig() ) );
    return make_var( SI64( ip->ptr_alig() ) );
}

Var Scope::parse_info( const Var *sf, int off, BinStreamReader bin ) {
    int n = bin.read_positive_integer();
    for( int i = 0; i < n; ++i ) {
        Var v = parse( sf, bin.read_offset() );
        std::cout << v;
        //if ( v.type->base_type )
        //    std::cout << " (type=" << *v.type << ")";
        std::cout << std::endl;
    }
    return Var();
}



#define DECL_IR_TOK( N ) Var Scope::parse_##N( const Var *sf, int off, BinStreamReader bin ) { TODO; return Var(); }
#include "../Ir/Decl_UnaryOperations.h"
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK

