#include "../System/BinStreamReader.h"
#include "../System/UsualStrings.h"
#include "../System/ReadFile.h"
#include "../System/RaiiSave.h"
#include "../Met/IrWriter.h"
#include "../Met/Lexer.h"
#include "Sourcefile.h"
#include <algorithm>
#include "Symbol.h"
#include "Select.h"
#include "Slice.h"
#include "Scope.h"
#include <limits>
#include "Cst.h"
#include "Def.h"
#include "Op.h"
#include "Ip.h"

#include "../Ir/AssignFlags.h"
#include "../Ir/Numbers.h"

Scope::Scope( Scope *parent, String name, Ip *_ip ) : ip( _ip ? _ip : ::ip ), parent( parent ) {
    if ( parent ) {
        path = parent->path + "/";
        if ( parent->self.defined() )
            self = parent->self;
    }
    path += name;

    static_scope = ip->get_static_scope( path );
    do_not_execute_anything = false;
    class_scope = 0;
    method = false;

    base_size = 0;
    base_alig = 1;

}

Var Scope::VecNamedVar::add( int name, Var var ) {
    NamedVar *res = data.push_back();
    res->name = name;
    res->var = var;
    res->sf  = ::ip->sf;
    res->off = ::ip->off;
    return res->var;
}

bool Scope::VecNamedVar::contains( int name ) {
    for( NamedVar &nv : data )
        if ( nv.name == name )
            return true;
    return false;
}

Var Scope::VecNamedVar::get( int name ) {
    for( NamedVar &nv : data )
        if ( nv.name == name )
            return nv.var;
    return Var();
}

void Scope::VecNamedVar::get( Vec<Var> &lst, int name ) {
    for( NamedVar &nv : data )
        if ( nv.name == name )
            lst << nv.var;
}

void Scope::import( String file ) {
    SourceFile *sf = ip->new_sf( file );
    if ( not sf )
        return;

    // -> source data
    ReadFile r( file.c_str() );
    if ( not r )
        return ip->disp_error( "Impossible to open " + file );

    // -> lexical data
    Lexer l( ip->error_list );
    l.parse( r.data, file.c_str() );
    if ( ip->error_list )
        return;

    IrWriter t( ip->error_list );
    t.parse( l.root() );
    if ( ip->error_list )
        return;

    // -> fill sf->tok_data
    sf->tok_data.resize( t.size_of_binary_data() );
    t.copy_binary_data_to( sf->tok_data.ptr() );

    // -> cor str
    BinStreamReader bin( sf->tok_data.ptr() );
    while ( true ) {
        int s = bin.read_positive_integer();
        if ( not s )
            break;
        sf->cor_str << ip->str_cor.num( String( bin.ptr, bin.ptr + s ) );
        bin.ptr += s;
    }

    // -> instructions
    ip->sf = sf;
    parse( bin.ptr );
}

Var Scope::parse( SourceFile *sf, const PI8 *tok, const char *reason ) {
    ip->push_sf( sf, reason );
    Var res = parse( tok );
    ip->pop_sf();
    return res;
}

Var Scope::parse( const PI8 *tok ) {
    if ( tok == 0 or do_not_execute_anything )
        return ip->error_var();

    BinStreamReader bin( tok );
    PI8 tva = bin.get<PI8>(); ///< token type
    ip->off = bin.read_positive_integer(); ///< offset in sourcefile

    switch ( tva ) {
        #define DECL_IR_TOK( N ) case IR_TOK_##N: return parse_##N( bin );
        #include "../Ir/Decl.h"
        #undef DECL_IR_TOK
        default: return ip->ret_error( "Unknown token type" );
    }
}

Var Scope::parse_BLOCK( BinStreamReader bin ) {
    Var res;
    while ( const PI8 *tok = bin.read_offset() )
        res = parse( tok );
    return res;
}

Var Scope::parse_CALLABLE( BinStreamReader bin, Type *type ) {
    int name = read_nstring( bin );

    // supporting variable
    Callable *c;
    switch( name ) {
    #define DECL_BT( T ) case STRING_##T##_NUM: c = &ip->class_##T; break;
    #include "DeclBaseClass.h"
    #include "DeclParmClass.h"
    #undef DECL_BT
    default:
        if ( type == &ip->type_Def ) c = new Def;
        else if ( type == &ip->type_Class ) c = new Class;
        else ERROR( "..." );
    }

    // fill in
    c->name = name;
    c->off  = ip->off;
    c->sf   = ip->sf;
    c->read_bin( bin );

    SI64 ptr = (SI64)c;
    Var res( type, cst( 64, (PI8 *)&ptr ) );
    res.flags |= Var::SURDEF;

    reg_var( name, res, true );
    return ip->void_var();
}

Var Scope::parse_DEF( BinStreamReader bin ) {
    return parse_CALLABLE( bin, &ip->type_Def );
}
Var Scope::parse_CLASS( BinStreamReader bin ) {
    return parse_CALLABLE( bin, &ip->type_Class );
}
Var Scope::parse_RETURN( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}

struct CmpCallableInfobyPertinence {
    bool operator()( const Callable *a, const Callable *b ) const {
        return a->pertinence > b->pertinence;
    }
};

Var Scope::apply( Var f, int nu, Var *u_args, int nn, int *n_name, Var *n_args, ApplyMode am ) {
    if ( f.type == &ip->type_Error )
        return ip->error_var();
    for( int i = 0; i < nu; ++i )
        if ( u_args[ i ].type == &ip->type_Error )
            return ip->error_var();
    for( int i = 0; i < nn; ++i )
        if ( n_args[ i ].type == &ip->type_Error )
            return ip->error_var();

    if ( f.type->orig == &ip->class_Callable ) {
        Var sur_var = f.type->parameters[ 0 ];
        SI32 nb_surdefs = sur_var.get_val()->size() / ip->type_ST->size();
        Callable *ci[ nb_surdefs ];
        Expr inst_data = sur_var.get_val();
        Type *va_type = sur_var.type;
        for( SI32 n = 0, o = 0; va_type != &ip->type_VarargsItemEnd; ++n, o += 64 ) {
            if ( n >= nb_surdefs or va_type->orig != &ip->class_VarargsItemBeg ) {
                PRINT( sur_var );
                PRINT( inst_data );
                PRINT( inst_data->size() );
                return ip->ret_error( "bad Callable" );
            }
            // ptr
            SI64 d;
            Expr p = slice( inst_data, o, 64 );
            if ( not p->_get_val( 64 )->get_val( &d, 64 ) )
                return ip->ret_error( "expecting a known Callable ptr" );
            // type (Def, Class, ...)
            ci[ n ] = reinterpret_cast<Callable *>( ST( d ) );
            // next
            va_type = ip->type_from_type_var( va_type->parameters[ 2 ] );
        }


        // self
        Var l_self;
        l_self.type = ip->type_from_type_var( f.type->parameters[ 1 ] );
        if ( l_self.type and l_self.type != &ip->type_Void ) {
            if ( l_self.type == &ip->type_Error )
                return ip->error_var();
            l_self.inst = slice( f.get_val(), 0, ip->type_ST->size() );
        }

        // parm
        Vec<int> pn_names;
        Vec<Var> pu_args, pn_args;
        Type *parm_type = ip->type_from_type_var( f.type->parameters[ 2 ] );
        if ( parm_type != &ip->type_Void ) {
            if ( parm_type->orig != &ip->class_VarargsItemBeg and parm_type->orig != &ip->class_VarargsItemEnd ) {
                PRINT( f );
                PRINT( *parm_type );
                TODO;
                return ip->ret_error( "expecting a vararg type (or void) as third arg of a callable type" );
            }

            int o = 0;
            Var vp = ( f.ptr() + 1 * ip->type_ST->size() ).at( ip->type_ST ); // pointer on varargs data
            while ( parm_type->orig != &ip->class_VarargsItemEnd ) {
                Var p_arg = ( vp + o * ip->type_ST->size() ).at( ip->type_ST );

                SI32 tn;
                if ( not parm_type->parameters[ 1 ].get_val( tn ) )
                    return ip->ret_error( "expecting a known SI32 as second arg of a varargs" );
                if ( tn >= 0 ) {
                    pn_args << Var( Ref(), ip->type_from_type_var( parm_type->parameters[ 0 ] ), p_arg.get_val() );
                    pn_names << tn;
                } else {
                    pu_args << Var( Ref(), ip->type_from_type_var( parm_type->parameters[ 0 ] ), p_arg.get_val() );
                }

                // iteration
                ++o;
                parm_type = ip->type_from_type_var( parm_type->parameters[ 2 ] );
                if ( parm_type->orig != &ip->class_VarargsItemBeg and parm_type->orig != &ip->class_VarargsItemEnd )
                    return ip->ret_error( "expecting a vararg type (or void) as third arg of a varargs" );
            }
        }
        int pnu = pu_args.size(), pnn = pn_args.size();

        // tests
        std::sort( ci, ci + nb_surdefs, CmpCallableInfobyPertinence() );

        int nb_ok = 0;
        double guaranted_pertinence = 0;
        bool   has_guaranted_pertinence = false;
        AutoPtr<Callable::Trial> trials[ nb_surdefs ];
        for( int i = 0; i < nb_surdefs; ++i ) {
            if ( has_guaranted_pertinence and guaranted_pertinence > ci[ i ]->pertinence ) {
                for( int j = i; j < nb_surdefs; ++j )
                    trials[ j ] = new Callable::Trial( "Already a more pertinent solution" );
                break;
            }

            trials[ i ] = ci[ i ]->test( nu, u_args, nn, n_name, n_args, pnu, pu_args.ptr(), pnn, pn_names.ptr(), pn_args.ptr(), l_self );

            if ( trials[ i ]->ok() ) {
                if ( trials[ i ]->cond.always( true ) ) {
                    has_guaranted_pertinence = true;
                    guaranted_pertinence = ci[ i ]->pertinence;
                }
                ++nb_ok;
            }
        }

        //
        for( int i = 0; i < nb_surdefs; ++i )
            if ( trials[ i ]->cond.type == &ip->type_Error )
                return ip->error_var();

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
            ErrorList::Error &err = ip->error_msg( ss.str() );
            for( int i = 0; i < nb_surdefs; ++i )
                err.ap( ci[ i ]->sf->name.c_str(), ci[ i ]->off, std::string( "possibility (" ) + trials[ i ]->reason + ")" );
            std::cerr << err;
            return ip->error_var();
        }

        // valid surdefs, but only with runtime conditions
        if ( not has_guaranted_pertinence ) {
            std::ostringstream ss;
            ss << "There is no failback surdefinition (only runtime conditions)";
            ErrorList::Error &err = ip->error_msg( ss.str() );
            for( int i = 0; i < nb_surdefs; ++i ) {
                if ( trials[ i ]->ok() )
                    err.ap( ci[ i ]->sf->name.c_str(), ci[ i ]->off, "accepted" );
                else
                    err.ap( ci[ i ]->sf->name.c_str(), ci[ i ]->off, std::string( "rejected (" ) + trials[ i ]->reason + ")" );
            }
            std::cerr << err;
            return ip->error_var();
        }

        // ambiguous overload ?
        if ( nb_ok > 1 ) {
            double best_pertinence = -std::numeric_limits<double>::max();
            for( int i = 0; i < nb_surdefs; ++i )
                best_pertinence = std::max( best_pertinence, ci[ i ]->pertinence );
            int nb_wp = 0;
            for( int i = 0; i < nb_surdefs; ++i )
                nb_wp += trials[ i ]->ok() and ci[ i ]->pertinence == best_pertinence;
            if ( nb_wp > 1 ) {
                std::ostringstream ss;
                ss << "Ambiguous overload";
                ErrorList::Error &err = ip->error_msg( ss.str() );
                for( int i = 0; i < nb_surdefs; ++i )
                    if ( trials[ i ]->ok() and ci[ i ]->pertinence == best_pertinence )
                        err.ap( ci[ i ]->sf->name.c_str(), ci[ i ]->off, "possibility" );
                std::cerr << err;
                return ip->error_var();
            }
        }

        Var res;
        Expr cond = ip->cond_stack.back();
        for( int i = 0; i < nb_surdefs; ++i ) {
            if ( trials[ i ]->ok() ) {
                if ( trials[ i ]->cond.defined() )
                    ip->set_cond( op( &ip->type_Bool, &ip->type_Bool, cond, &ip->type_Bool, trials[ i ]->cond.get_val(), Op_and_boolean() ) );
                else
                    ip->set_cond( cond );

                Var loc = trials[ i ]->call( nu, u_args, nn, n_name, n_args, pnu, pu_args.ptr(), pnn, pn_names.ptr(), pn_args.ptr(), l_self, am );
                res.set_val( loc );

                ip->pop_cond();

                if ( trials[ i ]->cond.defined() )
                    cond = op( &ip->type_Bool, &ip->type_Bool, cond, &ip->type_Bool, op( &ip->type_Bool, &ip->type_Bool, trials[ i ]->cond.get_val(), Op_not_boolean() ), Op_and_boolean() );
                else
                    break;
            }
        }

        return res;
    }

    //
    if ( f.type == &ip->type_Type ) {
        TODO;
//        TypeInfo *ti = ip->type_info( f.expr() ); ///< parse if necessary

//        Var ret;
//        if ( ti->static_size_in_bits >= 0 )
//            ret = Var( &ti->var, cst( 0, 0, ti->static_size_in_bits ) );
//        else
//            TODO; // res = undefined cst with unknown size

//        if ( am == APPLY_MODE_NEW )
//            TODO;

//        // call init
//        if ( am != APPLY_MODE_PARTIAL_INST )
//            apply( get_attr( ret, STRING_init_NUM, sf, off ), nu, u_args, nn, n_names, n_args, Scope::APPLY_MODE_STD, sf, off );
//        return ret;
    }

    // f.apply ...
    Var applier = get_attr( f, STRING_apply_NUM );
    if ( applier.is_an_error() )
        return applier;

    return apply( applier, nu, u_args, nn, n_name, n_args, am );
}

Var Scope::get_attr_rec( Var self, int name ) {
    // look in attributes
    if ( const Type::Attr *attr = self.type->find_attr( name ) )
        return self.type->make_attr( self, attr );

    // interet d'avoir super:
    //    // ancestors
    // OR NOT: may use aggr of attributes done in TypeInfo
    //    if ( name != STRING_init_NUM and name != STRING_destroy_NUM ) {
    //        for( int i = 0; i < self.type->ancestors.size(); ++i ) {
    //            Var super = self.type->make_attr( self, self.type->ancestor_attr( i ) );
    //            if ( Var res = get_attr_rec( super, name ) )
    //                return res;
    //        }
    //    }
    return Var();
}

void Scope::get_attr_rec( Vec<Var> &res, Var self, int name ) {
    // look in attributes
    Vec<const Type::Attr *> lattr;
    self.type->find_attr( lattr, name );
    for( const Type::Attr *attr: lattr )
        res << self.type->make_attr( self, attr );

    // interet d'avoir super:
    //    // ancestors
    // OR NOT: may use aggr of attributes done in TypeInfo
    //    if ( name != STRING_init_NUM and name != STRING_destroy_NUM ) {
    //        for( int i = 0; i < self.type->ancestors.size(); ++i ) {
    //            Var super = self.type->make_attr( self, self.type->ancestor_attr( i ) );
    //            if ( Var res = get_attr_rec( super, name ) )
    //                return res;
    //        }
    //    }
}

Var Scope::copy( Var &var ) {
    // shortcut (for bootstrap)
    if ( var.type->pod() )
        return Var( var.type, var.get_val() );
    //
    Var res( var.type );
    apply( get_attr( res, STRING_init_NUM ), 1, &var );
    return res;
}

Var Scope::get_attr( Var self, int name ) {
    if ( self.type == &ip->type_Error )
        return ip->error_var();

    if ( not self.type->orig )
        return ip->ret_error( "Class " + to_string( *self.type ) + " has not been defined" );

    if ( self.type->orig == &ip->class_GetSetSopInst )
        TODO;
        //return get_attr( get_val_if_GetSetSopInst( self, sf, off ), name, sf, off );

    // in self or super(s)
    Var res = get_attr_rec( self, name );

    // look for external methods
    if ( res.type == 0 ) {
        for( Scope *s = this; s; s = s->parent ) {
            if ( s->static_scope ) {
                Var tmp = s->static_scope->get( name );
                if ( tmp.defined() ) {
                    if ( tmp.type == &ip->type_Def ) {
                        res = tmp;
                        break;
                    }
                }
            }
        }
    }

    if ( res.type == 0 )
        return ip->ret_error( "no attr '" + ip->str_cor.str( name ) + "' in object of type '" + to_string( *self.type ) + "' or in parent scopes" );


    if ( res.type->orig == &ip->class_GetSetSopDef ) {
        TODO;
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
    }


    if ( res.is_surdef() ) {
        // find all the variants
        Vec<Var> lst;

        // in attributes
        get_attr_rec( lst, self, name );

        // (filtered) external methods
        int os = lst.size();
        for( Scope *s = this; s; s = s->parent )
            if ( s->static_scope )
                s->static_scope->get( lst, name );
        for( int i = os; i < lst.size(); ++i )
            if ( not ip->ext_method( lst[ i ] ) )
                lst.remove_unordered( i-- );

        return ip->make_Callable( lst, self );
    }

    return res;
}

Var Scope::parse_APPLY( BinStreamReader bin ) {
    Var f = parse( bin.read_offset() );

    int nu = bin.read_positive_integer();
    Var u_args[ nu ];
    for( int i = 0; i < nu; ++i )
        u_args[ i ] = parse( bin.read_offset() );

    int nn = bin.read_positive_integer();
    int n_name[ nn ];
    Var n_args[ nn ];
    for( int i = 0; i < nn; ++i ) {
        n_name[ i ] = read_nstring( bin );
        n_args[ i ] = parse( bin.read_offset() );
    }

    return apply( f, nu, u_args, nn, n_name, n_args, APPLY_MODE_STD );
}
Var Scope::parse_SELECT( BinStreamReader bin ) {
    Var f = parse( bin.read_offset() );

    int nu = bin.read_positive_integer();
    Var u_args[ nu ];
    for( int i = 0; i < nu; ++i )
        u_args[ i ] = parse( bin.read_offset() );

    int nn = bin.read_positive_integer();
    int n_name[ nn ];
    Var n_args[ nn ];
    for( int i = 0; i < nn; ++i ) {
        n_name[ i ] = read_nstring(  bin );
        n_args[ i ] = parse( bin.read_offset() );
    }

    // shortcut for Callable
    if ( f.type->orig == &ip->class_Callable ) {
        // varargs to gather the arguments
        Vec<Var> vs;
        Vec<int> ns;

        Type *t = ip->type_from_type_var( f.type->parameters[ 2 ] );
        if ( t != &ip->type_Void )
            TODO;

        for( int i = 0; i < nn; ++i ) {
            vs << n_args[ i ];
            ns << n_name[ i ];
        }
        for( int i = 0; i < nu; ++i )
            vs << u_args[ i ];
        Var va = ip->make_Varargs( vs, ns );

        // new type
        Vec<Var> lt;
        lt << f.type->parameters[ 0 ];
        lt << f.type->parameters[ 1 ];
        lt << ip->make_type_var( va.type );

        Type *t_res = ip->class_Callable.type_for( lt );
        t_res->_len = 2 * ip->type_ST->size();

        // new data
        Var res( t_res, f.get_val() );
        res.set_val( ip->type_ST->size(), va.ptr() );

        return res;
    }

    //
    return apply( get_attr( f, STRING_select_NUM ), nu, u_args, nn, n_name, n_args, APPLY_MODE_STD );
}
Var Scope::parse_CHBEBA( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_SI32( BinStreamReader bin ) {
    SI32 data = bin.read_positive_integer();
    return Var( &ip->type_SI32, cst( 32, (PI8 *)&data ) );
}
Var Scope::parse_PI32( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_SI64( BinStreamReader bin ) {
    SI64 data = bin.read_positive_integer();
    return Var( &ip->type_SI64, cst( 64, (PI8 *)&data ) );
}
Var Scope::parse_PI64( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_PTR( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_STRING( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_VAR( BinStreamReader bin ) {
    int name = read_nstring( bin );
    Var res = find_var( name );
    if ( res.defined() )
        return res;
    return ip->ret_error( "Impossible to find variable '" + ip->str_cor.str( name ) + "'." );
}
Var Scope::find_first_var( int name ) {
    if ( self.defined() ) {
        Var res = get_attr_rec( self, name );
        if ( res.defined() )
            return res;
    }
    for( Scope *s = this; s; s = s->parent ) {
        Var res = s->local_scope.get( name );
        if ( res.defined() )
            return res;
        res = s->static_scope->get( name );
        if ( res.defined() )
            return res;
    }
    return Var();
}

void Scope::find_var_clist( Vec<Var> &lst, int name ) {
    if ( self.defined() )
        get_attr_rec( lst, self, name );
    for( Scope *s = this; s; s = s->parent ) {
        s->local_scope.get( lst, name );
        s->static_scope->get( lst, name );
    }
}

Var Scope::find_var( int name ) {
    Var res = find_first_var( name );
    if ( res.is_surdef() ) {
        // surdef_list = nb_surdefs, [ surdef_expr ]*n
        Vec<Var> lst;
        find_var_clist( lst, name );
        return ip->make_Callable( lst, self );
    }
    return res;

}

int Scope::read_nstring( BinStreamReader &bin ) {
    return ip->sf->cor_str[ bin.read_positive_integer() ];
}

Var Scope::parse_ASSIGN( BinStreamReader bin ) {
    // name
    int name = read_nstring( bin );

    // flags
    int flags = bin.read_positive_integer();

    // inst
    Var var = parse( bin.read_offset() );

    //
    if ( flags & IR_ASSIGN_TYPE )
        TODO;
        // var = apply( var, 0, 0, 0, 0, 0, class_scope ? APPLY_MODE_PARTIAL_INST : APPLY_MODE_STD, sf, off );

    //if ( ( flags & IR_ASSIGN_REF ) == 0 and var.referenced_more_than_one_time() )
    //    var = copy( var, sf, off );

    if ( flags & IR_ASSIGN_CONST ) {
        TODO;
        //        if ( ( var.flags & Var::WEAK_CONST ) == 0 and var.referenced_more_than_one_time() )
        //            disp_error( "Impossible to make this variable fully const (already referenced elsewhere)", sf, off );
        //        var.flags |= Var::WEAK_CONST;
        //        var.data->flags = PRef::CONST;
    }

    return reg_var( name, var, flags & IR_ASSIGN_STATIC );
}
Var Scope::reg_var( int name, Var var, bool stat ) {
    if ( not var.is_surdef() and ( local_scope.contains( name ) or static_scope->contains( name ) ) )
        return ip->ret_error( "There is already a var named '" + ip->str_cor.str( name ) + "' in the current scope" );
    VecNamedVar &scope = stat ? *static_scope : local_scope;
    return scope.add( name, var );
}

Var Scope::parse_REASSIGN( BinStreamReader bin ) {
    Var a = parse( bin.read_offset() );
    Var b = parse( bin.read_offset() );
    return apply( get_attr( a, STRING_reassign_NUM ), 1, &b, 0, 0, 0, APPLY_MODE_STD );
}
Var Scope::parse_GET_ATTR( BinStreamReader bin ) {
    Var self = parse( bin.read_offset() );
    int attr = read_nstring( bin );
    if ( self.type == &ip->type_Error )
        return ip->error_var();
    Var res = get_attr( self, attr );
    if ( res.defined() )
        return res;
    return ip->ret_error( "No attribute " + ip->str_cor.str( attr ) );
}
Var Scope::parse_GET_ATTR_PTR( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_GET_ATTR_ASK( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_GET_ATTR_PTR_ASK( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_GET_ATTR_PA( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_IF( BinStreamReader bin ) {
    Var cond = parse( bin.read_offset() );
    if ( cond.type == &ip->type_Error )
        return cond;

    // bool conversion
    if ( cond.type != &ip->type_Bool ) {
        TODO;
        //        cond = apply( find_var( STRING_Bool_NUM ), 1, &cond, 0, 0, 0, APPLY_MODE_STD, sf, off );
        //        if ( ip->isa_Error( cond ) )
        //            return cond;
    }

    // simplified expression
    Expr expr = cond.get_val();

    //
    const PI8 *ok = bin.read_offset();
    const PI8 *ko = bin.read_offset();

    // known value
    bool cond_val;
    if ( expr->get_val( cond_val ) ) {
        if ( cond_val ) {
            Scope if_scope( this, "if_" + to_string( ok ) );
            return if_scope.parse( ok );
        }
        // else
        if ( ko ) {
            Scope if_scope( this, "fi_" + to_string( ko ) );
            return if_scope.parse( ko );
        }
        return ip->void_var();
    }

    Var res_ok;
    if ( ok ) {
        ip->set_cond( expr );
        Scope if_scope( this, "if_" + to_string( ok ) );
        res_ok = if_scope.parse( ok );
        ip->pop_cond();
    }

    Var res_ko;
    if ( ko ) {
        ip->set_cond( op( &ip->type_Bool, &ip->type_Bool, expr, Op_not_boolean() ) );
        Scope if_scope( this, "fi_" + to_string( ko ) );
        res_ko = if_scope.parse( ko );
        ip->pop_cond();
    }

    if ( res_ok.type != res_ko.type or not ko )
        return ip->void_var();
    return Var( Ref(), res_ok.type, select( expr, res_ok.ref(), res_ko.ref() ) );
}

Var Scope::parse_WHILE( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_BREAK( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_CONTINUE( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_FALSE( BinStreamReader bin ) {
    Bool val = false;
    return Var( &ip->type_Bool, cst( 1, (PI8 *)&val ) );
}
Var Scope::parse_TRUE( BinStreamReader bin ) {
    Bool val = true;
    return Var( &ip->type_Bool, cst( 1, (PI8 *)&val ) );
}
Var Scope::parse_VOID( BinStreamReader bin ) {
    return Var( &ip->type_Void );
}
Var Scope::parse_SELF( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_THIS( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_FOR( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_IMPORT( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_NEW( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_LIST( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_LAMBDA( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_NULL_REF( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_AND( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_OR( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}

Var Scope::parse_info( BinStreamReader bin ) {
    int n = bin.read_positive_integer();
    for( int i = 0; i < n; ++i )
        std::cout << ( i ? ", " : "" ) << parse( bin.read_offset() );
    std::cout << std::endl;
    return ip->void_var();
}
Var Scope::parse_disp( BinStreamReader bin ) {
    int n = bin.read_positive_integer();
    for( int i = 0; i < n; ++i )
        std::cout << ( i ? ", " : "" ) << parse( bin.read_offset() ).get_val();
    std::cout << std::endl;
    return ip->void_var();
}
Var Scope::parse_rand( BinStreamReader bin ) {
    return Var( &ip->type_Bool, symbol( "rand", 1 ) );
}
Var Scope::parse_syscall( BinStreamReader bin ) {
    int n = bin.read_positive_integer();
    Vec<Var> inp( Rese(), n );
    for( int i = 0; i < n; ++i )
        inp << parse( bin.read_offset() );

    return syscall( inp );
}

#define CHECK_PRIM_ARGS( N ) \
    int n = bin.read_positive_integer(); \
    if ( n != N ) \
        return ip->ret_error( "Expecting " #N " operand" );

Var Scope::parse_set_base_size_and_alig( BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 2 );
    Var a = parse( bin.read_offset() );
    Var b = parse( bin.read_offset() );
    if ( a.type == &ip->type_Error or b.type == &ip->type_Error )
        return ip->void_var();
    if ( not a.get_val( base_size ) or not b.get_val( base_alig ) ) {
        PRINT( a );
        PRINT( b );
        return ip->ret_error( "set_base_size_and_alig -> SI32/SI64 known values" );
    }
    return ip->void_var();
}
Var Scope::parse_set_RawRef_dependancy( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_reassign_rec( BinStreamReader bin ) {
    int n = bin.read_positive_integer();
    if ( n == 1 ) {
        if ( not self.defined() )
            return ip->ret_error( "expecting a defined self" );
        self.set_val( parse( bin.read_offset() ) );
        return self;
    }
    if ( n != 2 )
        return ip->ret_error( "expecting 1 or 2 args" );
    Var dst = parse( bin.read_offset() );
    Var src = parse( bin.read_offset() );
    dst.set_val( src );
    return dst;
}
Var Scope::parse_assign_rec( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_set_ptr_val( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_select_SurdefList( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_ptr_size( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_ptr_alig( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_size_of( BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 1 );
    Var T = parse( bin.read_offset() );
    if ( Type *t = ip->type_from_type_var( T ) )
        return Var( ip->type_ST, cst( ST( t->size() ) ) );
    return ip->ret_error( "Expecting a type variable" );
}
Var Scope::parse_alig_of( BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 1 );
    Var T = parse( bin.read_offset() );
    if ( Type *t = ip->type_from_type_var( T ) )
        return Var( ip->type_ST, cst( ST( t->alig() ) ) );
    return ip->ret_error( "Expecting a type variable" );
}
Var Scope::parse_typeof( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_address( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_get_slice( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_pointed_value( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_pointer_on( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_block_exec( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_get_argc( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_get_argv( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_apply_LambdaFunc( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Var Scope::parse_inst_of( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}

template<class OP>
Type *Scope::type_promote( Type *ta, Type *tb, OP ) {
    if ( OP::b )
        return &ip->type_Bool;
    if ( ta == tb )
        return ta;
    TODO;
    return 0;
}

template<class OP>
Var Scope::parse_una( BinStreamReader bin, OP o ) {
    CHECK_PRIM_ARGS( 1 );
    TODO;
    return Var();
}

template<class OP>
Var Scope::parse_bin( BinStreamReader bin, OP o ) {
    CHECK_PRIM_ARGS( 2 );
    Var a = parse( bin.read_offset() );
    Var b = parse( bin.read_offset() );
    Type *tr = type_promote( a.type, b.type, o );
    return Var( tr, op( tr, a.type, a.get_val(), b.type, b.get_val(), o ) );
}

#define DECL_IR_TOK( N ) Var Scope::parse_##N( BinStreamReader bin ) { return parse_una( bin, Op_##N() ); }
#include "../Ir/Decl_UnaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( N ) Var Scope::parse_##N( BinStreamReader bin ) { return parse_bin( bin, Op_##N() ); }
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK
