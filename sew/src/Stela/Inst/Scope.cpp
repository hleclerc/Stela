#include "../System/BinStreamReader.h"
#include "../System/UsualStrings.h"
#include "../System/ReadFile.h"
#include "../System/RaiiSave.h"
#include "../System/AutoPtr.h"
#include "../Ir/CallableFlags.h"
#include "../Ir/AssignFlags.h"
#include "../Ir/Numbers.h"
#include "../Met/IrWriter.h"
#include "../Met/Lexer.h"
#include "Sourcefile.h"
#include <algorithm>
#include "ReplBits.h"
#include "Syscall.h"
#include "Symbol.h"
#include "Select.h"
#include "Scope.h"
#include "Slice.h"
#include "Class.h"
#include "Room.h"
#include "Type.h"
#include "Def.h"
#include "Cst.h"
#include "Op.h"
#include "Ip.h"

Scope::Scope( Scope *parent, Scope *caller, String name, Ip *lip ) :
    parent( parent ), caller( caller ) {
    if ( parent ) {
        path = parent->path + "/";
        self = parent->self;
    }
    path += name;

    static_vars = ( lip ? lip : ip )->get_static_vars( path );
    do_not_execute_anything = false;
    class_scope = 0;
    method = false;


    base_size = 0;
    base_alig = 1;

    sf     = 0;
    off    = 0;
    reason = 0;
}

Expr Scope::import( String file ) {
    SourceFile *sf = ip->new_sf( file );
    if ( not sf )
        return ip->error_var();

    // -> source data
    ReadFile r( file.c_str() );
    if ( not r )
        return ip->ret_error( "Impossible to open " + file );

    // -> lexical data
    Lexer l( ip->error_list );
    l.parse( r.data, file.c_str() );
    if ( ip->error_list )
        return ip->error_var();

    IrWriter t( ip->error_list );
    t.parse( l.root() );
    if ( ip->error_list )
        return ip->error_var();

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
    return parse( sf, bin.ptr, "import" );
}

Expr Scope::parse( SourceFile *nsf, const PI8 *ntok, const char *nreason ) {
    auto oreason = raii_save( reason, nreason );
    auto osf     = raii_save( sf    , nsf );
    auto ooff    = raii_save( off );
    return parse( ntok );
}

Expr Scope::parse( const PI8 *tok ) {
    if ( tok == 0 or do_not_execute_anything )
        return ip->error_var();

    BinStreamReader bin( tok );
    PI8 tva = bin.get<PI8>(); ///< token type
    off = bin.read_positive_integer(); ///< offset in sourcefile
    auto rs = raii_save( ip->cur_scope, this );

    switch ( tva ) {
        #define DECL_IR_TOK( N ) case IR_TOK_##N: return parse_##N( bin );
        #include "../Ir/Decl.h"
        #undef DECL_IR_TOK
        default: return ip->ret_error( "Unknown token type" );
    }
}

int Scope::read_nstring( BinStreamReader &bin ) {
    return sf->cor_str[ bin.read_positive_integer() ];
}

Expr Scope::parse_BLOCK( BinStreamReader bin ) {
    Expr res;
    while ( const PI8 *tok = bin.read_offset() )
        res = parse( tok );
    return res;
}

Expr Scope::parse_CALLABLE( BinStreamReader bin, Class *base_class ) {
    int name = read_nstring( bin );

    // supporting variable
    Callable *c = 0;
    if ( base_class == ip->class_Class ) {
        switch( name ) {
        #define DECL_BT( T ) case STRING_##T##_NUM: c = ip->class_##T; break;
        #include "DeclBaseClass.h"
        #include "DeclParmClass.h"
        #undef DECL_BT
        default: break;
        }
    }
    if ( not c ) {
        if      ( base_class == ip->class_Def   ) c = new Def;
        else if ( base_class == ip->class_Class ) c = new Class;
        else ERROR( "..." );
    }

    // fill in (read bin data)
    c->name = name;
    c->off  = off;
    c->sf   = sf;
    c->read_bin( this, bin );

    // get catched vars
    Vec<Expr> catched_vars;
    for( int i = 0; i < c->catched_vars.size(); ++i ) {
        Callable::CatchedVar &cv = c->catched_vars[ i ];
        Expr res = 125;
        switch ( cv.type ) {
        case IN_LOCAL_SCOPE : TODO;
        case IN_STATIC_SCOPE: TODO;
        case IN_CATCHED_VARS: TODO;
        default: ERROR( "???" );
        }
        catched_vars << res;
    }
    Expr cva = ip->make_Varargs( catched_vars );

    // output type
    // Def[ catched_vars_type ]
    //   cpp_inst_ptr (inst in C++ of Def or Class)
    //   catched_var_ptr
    Vec<Expr> lt;
    lt << ip->make_type_var( cva->type() );
    Type *type = base_class->type_for( lt );
    type->_len = 64 + ip->type_ST->size();
    type->_pod = 1;

    // output val
    Expr val = cst( type, 64 + ip->type_ST->size() );

    SI64 ptr = (SI64)c;
    val = repl_bits( val,  0, cst( ip->type_ST, 64, &ptr ) );
    val = repl_bits( val, 64, cva );

    // output var
    Expr res = room( val, Inst::SURDEF | Inst::CONST );

    if ( parent ) local_vars << res;
    else ip->vars.add( name, res );
    return res;
}

Expr Scope::parse_DEF( BinStreamReader bin ) {
    return parse_CALLABLE( bin, ip->class_Def );
}

Expr Scope::parse_CLASS( BinStreamReader bin ) {
    return parse_CALLABLE( bin, ip->class_Class );
}

Expr Scope::parse_RETURN( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}

struct CmpCallableInfobyPertinence {
    bool operator()( const Callable *a, const Callable *b ) const {
        return a->pertinence > b->pertinence;
    }
};

Expr Scope::apply( Expr f, int nu, Expr *u_args, int nn, int *n_name, Expr *n_args, ApplyMode am ) {
    if ( f.error() )
        return ip->error_var();
    for( int i = 0; i < nu; ++i )
        if ( u_args[ i ].error() )
            return ip->error_var();
    for( int i = 0; i < nn; ++i )
        if ( n_args[ i ].error() )
            return ip->error_var();

    // SurdefList( ... )
    Type *f_type = f->ptype();
    if ( f_type->orig == ip->class_SurdefList ) {
        Type *l_type = ip->type_from_type_var( f_type->parameters[ 0 ] );
        Expr lst = slice( l_type, f->get(), 0 )->get( cond );
        Type *vt = lst->type();
        int o = 0;
        Vec<Callable *> ci;
        // Vec<Expr> catched_vars;
        for ( ; vt->orig == ip->class_VarargsItemBeg; vt = ip->type_from_type_var( vt->parameters[ 2 ] ), o += ip->type_ST->size() ) {
            Type *pt = ip->type_from_type_var( vt->parameters[ 0 ] );
            Expr callable = slice( pt, lst, o )->get( cond );
            // Callable *
            Expr cptr = slice( ip->type_SI64, callable, 0 );
            SI64 cptr_val;
            if ( not cptr->get_val( ip->type_SI64, &cptr_val ) )
                return ip->ret_error( "exp. cst" );
            ci << reinterpret_cast<Callable *>( ST( cptr_val ) );
            // catched_var
            // catched_vars << slice( ip->type_from_type_var( callable->type()->parameters[ 0 ] ), callable, 64 );
        }

        // parm
        Vec<int> pn_names;
        Vec<Expr> pu_args, pn_args;
        Type *parm_type = ip->type_from_type_var( f_type->parameters[ 1 ] );
        if ( parm_type != ip->type_Void ) {
            TODO;
            //            if ( parm_type->orig != &ip->class_VarargsItemBeg and parm_type->orig != &ip->class_VarargsItemEnd ) {
            //                PRINT( f );
            //                PRINT( *parm_type );
            //                TODO;
            //                return ip->ret_error( "expecting a vararg type (or void) as third arg of a callable type" );
            //            }

            //            int o = 0;
            //            Expr vp = ( f.ptr() + 1 * ip->type_ST->size() ).at( ip->type_ST ); // pointer on varargs data
            //            while ( parm_type->orig != &ip->class_VarargsItemEnd ) {
            //                Expr p_arg = ( vp + o * ip->type_ST->size() ).at( ip->type_ST );

            //                SI32 tn;
            //                if ( not parm_type->parameters[ 1 ].get_val( tn ) )
            //                    return ip->ret_error( "expecting a known SI32 as second arg of a varargs" );
            //                if ( tn >= 0 ) {
            //                    pn_args << Var( Ref(), ip->type_from_type_var( parm_type->parameters[ 0 ] ), p_arg.get_val() );
            //                    pn_names << tn;
            //                } else {
            //                    pu_args << Var( Ref(), ip->type_from_type_var( parm_type->parameters[ 0 ] ), p_arg.get_val() );
            //                }

            //                // iteration
            //                ++o;
            //                parm_type = ip->type_from_type_var( parm_type->parameters[ 2 ] );
            //                if ( parm_type->orig != &ip->class_VarargsItemBeg and parm_type->orig != &ip->class_VarargsItemEnd )
            //                    return ip->ret_error( "expecting a vararg type (or void) as third arg of a varargs" );
            //            }
        }
        int pnu = pu_args.size(), pnn = pn_args.size();

        // tests
        std::sort( ci.begin(), ci.end(), CmpCallableInfobyPertinence() );

        int nb_ok = 0, nb_surdefs = ci.size();
        double guaranted_pertinence = 0;
        bool has_guaranted_pertinence = false;
        AutoPtr<Callable::Trial> trials[ nb_surdefs ];
        for( int i = 0; i < nb_surdefs; ++i ) {
            if ( has_guaranted_pertinence and guaranted_pertinence > ci[ i ]->pertinence ) {
                for( int j = i; j < nb_surdefs; ++j )
                    trials[ j ] = new Callable::Trial( "Already a more pertinent solution" );
                break;
            }

            trials[ i ] = ci[ i ]->test( nu, u_args, nn, n_name, n_args, pnu, pu_args.ptr(), pnn, pn_names.ptr(), pn_args.ptr(), this );

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
            if ( trials[ i ]->cond.error() )
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

        Expr res = room();
        BoolOpSeq cond = this->cond;
        for( int i = 0; i < nb_surdefs; ++i ) {
            if ( trials[ i ]->ok() ) {
                Expr loc = trials[ i ]->call( nu, u_args, nn, n_name, n_args, pnu, pu_args.ptr(), pnn, pn_names.ptr(), pn_args.ptr(), am, this );
                res->set( loc, cond and trials[ i ]->cond );

                if ( trials[ i ]->cond.always( true ) )
                    break;
                cond = cond and not trials[ i ]->cond;
            }
        }

        return res;
    }

    //
    if ( f_type == ip->type_Type ) {
        TODO;
//        TypeInfo *ti = ip->type_info( f.expr() ); ///< parse if necessary

//        Expr ret;
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
    Expr applier = get_attr( f, STRING_apply_NUM );
    if ( applier.error() )
        return ip->error_var();

    return apply( applier, nu, u_args, nn, n_name, n_args, am );
}

Expr Scope::get_attr_rec( Expr self, int name ) {
    if ( not self )
        return (Inst *)0;
    TODO;
    return 0;
//    // look in attributes
//    if ( const Type::Attr *attr = self.type->find_attr( name ) )
//        return self.type->make_attr( self, attr );

//    // interet d'avoir super:
//    //    // ancestors
//    // OR NOT: may use aggr of attributes done in TypeInfo
//    //    if ( name != STRING_init_NUM and name != STRING_destroy_NUM ) {
//    //        for( int i = 0; i < self.type->ancestors.size(); ++i ) {
//    //            Expr super = self.type->make_attr( self, self.type->ancestor_attr( i ) );
//    //            if ( Expr res = get_attr_rec( super, name ) )
//    //                return res;
//    //        }
//    //    }
    return (Inst *)0;
}

void Scope::get_attr_rec( Vec<Expr> &res, Expr self, int name ) {
    TODO;
//    // look in attributes
//    Vec<const Type::Attr *> lattr;
//    self.type->find_attr( lattr, name );
//    for( const Type::Attr *attr: lattr )
//        res << self.type->make_attr( self, attr );

//    // interet d'avoir super:
//    //    // ancestors
//    // OR NOT: may use aggr of attributes done in TypeInfo
//    //    if ( name != STRING_init_NUM and name != STRING_destroy_NUM ) {
//    //        for( int i = 0; i < self.type->ancestors.size(); ++i ) {
//    //            Expr super = self.type->make_attr( self, self.type->ancestor_attr( i ) );
//    //            if ( Expr res = get_attr_rec( super, name ) )
//    //                return res;
//    //        }
//    //    }
}

Expr Scope::copy( Expr &var ) {
    // shortcut (for bootstrap)
    if ( var->type()->pod() )
        return room( var->get( cond ) );
    //
    Expr res = cst( var->type(), var->size() );
    apply( get_attr( res, STRING_init_NUM ), 1, &var );
    return res;
}

Expr Scope::get_attr( Expr self, int name ) {
    TODO;
    return 0;
//    if ( self.error() )
//        return ip->error_var();

//    if ( not self.type->orig )
//        return ip->ret_error( "Class " + to_string( *self.type ) + " has not been defined" );

//    if ( self.type->orig == &ip->class_GetSetSopInst )
//        TODO;
//        //return get_attr( get_val_if_GetSetSopInst( self, sf, off ), name, sf, off );

//    // in self or super(s)
//    Expr res = get_attr_rec( self, name );

//    // look for external methods
//    if ( res.type == 0 ) {
//        for( Scope *s = this; s; s = s->parent ) {
//            if ( s->static_scope ) {
//                Expr tmp = s->static_scope->get( name );
//                if ( tmp.defined() ) {
//                    if ( tmp.type == &ip->type_Def ) {
//                        res = tmp;
//                        break;
//                    }
//                }
//            }
//        }
//    }

//    if ( res.type == 0 )
//        return ip->ret_error( "no attr '" + ip->str_cor.str( name ) + "' in object of type '" + to_string( *self.type ) + "' or in parent scopes" );


//    if ( res.type->orig == &ip->class_GetSetSopDef ) {
//        TODO;
//        //            SI32 get_of = to_int( res.type->parameters[ 0 ] );
//        //            Expr getr;
//        //            if ( get_of >= 0 )
//        //                getr = apply( get_attr( self, get_of, sf, off ), 0, 0, 0, 0, 0, true );

//        //            Vec<Var> par;
//        //            par << make_type_var( self.type );
//        //            par << make_type_var( getr.type );
//        //            for( int i = 0; i < 3; ++i )
//        //                par << res.type->parameters[ i ];
//        //            Expr tmp( ip, ip->class_GetSetSopInst.type_for( this, par, sf, off ) );
//        //            ip->set_ref( tmp.data + 0 * sizeof( void * ), self );
//        //            ip->set_ref( tmp.data + 1 * sizeof( void * ), getr );
//        //            return tmp;
//    }


//    if ( res.is_surdef() ) {
//        // find all the variants
//        Vec<Var> lst;

//        // in attributes
//        get_attr_rec( lst, self, name );

//        // (filtered) external methods
//        int os = lst.size();
//        for( Scope *s = this; s; s = s->parent )
//            if ( s->static_scope )
//                s->static_scope->get( lst, name );
//        for( int i = os; i < lst.size(); ++i )
//            if ( not ip->ext_method( lst[ i ] ) )
//                lst.remove_unordered( i-- );

//        return ip->make_SurdefList( lst );
//    }

//    return res;
}

Expr Scope::parse_APPLY( BinStreamReader bin ) {
    Expr f = parse( bin.read_offset() );

    int nu = bin.read_positive_integer();
    Expr u_args[ nu ];
    for( int i = 0; i < nu; ++i )
        u_args[ i ] = parse( bin.read_offset() );

    int nn = bin.read_positive_integer();
    int n_name[ nn ];
    Expr n_args[ nn ];
    for( int i = 0; i < nn; ++i ) {
        n_name[ i ] = read_nstring( bin );
        n_args[ i ] = parse( bin.read_offset() );
    }

    return apply( f, nu, u_args, nn, n_name, n_args, APPLY_MODE_STD );
}

Expr Scope::parse_SELECT( BinStreamReader bin ) {
    Expr f = parse( bin.read_offset() );

    int nu = bin.read_positive_integer();
    Expr u_args[ nu ];
    for( int i = 0; i < nu; ++i )
        u_args[ i ] = parse( bin.read_offset() );

    int nn = bin.read_positive_integer();
    int n_name[ nn ];
    Expr n_args[ nn ];
    for( int i = 0; i < nn; ++i ) {
        n_name[ i ] = read_nstring(  bin );
        n_args[ i ] = parse( bin.read_offset() );
    }

    // shortcut for Callable
    if ( f->type()->orig == ip->class_SurdefList ) {
        TODO;
        return 0;
//        // varargs to gather the arguments
//        Vec<Var> vs;
//        Vec<int> ns;

//        Type *t = ip->type_from_type_var( f.type->parameters[ 2 ] );
//        if ( t != &ip->type_Void )
//            TODO;

//        for( int i = 0; i < nn; ++i ) {
//            vs << n_args[ i ];
//            ns << n_name[ i ];
//        }
//        for( int i = 0; i < nu; ++i )
//            vs << u_args[ i ];
//        Expr va = ip->make_Varargs( vs, ns );

//        // new type
//        Vec<Var> lt;
//        lt << f.type->parameters[ 0 ];
//        lt << f.type->parameters[ 1 ];
//        lt << ip->make_type_var( va.type );

//        Type *t_res = ip->class_SurdefList.type_for( lt );
//        t_res->_len = 2 * ip->type_ST->size();

//        // new data
//        Expr res( t_res, f.get_val() );
//        res.set_val( ip->type_ST->size(), va.ptr(), Rese(), Expr() );

//        return res;
    }

    //
    return apply( get_attr( f, STRING_select_NUM ), nu, u_args, nn, n_name, n_args, APPLY_MODE_STD );
}
Expr Scope::parse_CHBEBA( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_SI32( BinStreamReader bin ) {
    SI32 data = bin.read_positive_integer();
    return room( cst( ip->type_SI32, 32, &data ) );
}
Expr Scope::parse_PI32( BinStreamReader bin ) {
    PI32 data = bin.read_positive_integer();
    return room( cst( ip->type_PI32, 32, &data ) );
}
Expr Scope::parse_SI64( BinStreamReader bin ) {
    SI64 data = bin.read_positive_integer();
    return room( cst( ip->type_SI64, 64, &data ) );
}
Expr Scope::parse_PI64( BinStreamReader bin ) {
    PI64 data = bin.read_positive_integer();
    return room( cst( ip->type_PI64, 64, &data ) );
}
Expr Scope::parse_PTR( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_STRING( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}

Expr Scope::parse_VAR( BinStreamReader bin ) {
    int name = read_nstring( bin );
    if ( Expr res = find_var( name ) )
        return res;
    return ip->ret_error( "Impossible to find variable '" + ip->str_cor.str( name ) + "'." );
}

Expr Scope::parse_VAR_IN_LOCAL_SCOPE( BinStreamReader bin ) {
    int np = bin.read_positive_integer();
    int ns = bin.read_positive_integer();
    Scope *s = this;
    for( ; np; --np )
        s = s->parent;
    if ( not s )
        return ip->ret_error( "bad np" );
    if ( ns >= s->local_vars.size() )
        return ip->ret_error( "bad ns" );
    return s->local_vars[ ns ];
}

Expr Scope::parse_VAR_IN_STATIC_SCOPE( BinStreamReader bin ) {
    TODO;
    return 0;
}

Expr Scope::parse_VAR_IN_CATCHED_VARS( BinStreamReader bin ) {
    TODO;
    return 0;
}

Expr Scope::find_first_var( int name ) {
    if ( Expr res = ip->vars.get( name ) )
        return res;
    return (Inst *)0;
}

void Scope::find_var_clist( Vec<Expr> &lst, int name ) {
    ip->vars.get( lst, name );
}

Expr Scope::find_var( int name ) {
    Expr res = find_first_var( name );
    if ( res and res->is_surdef() ) {
        Vec<Expr> lst;
        find_var_clist( lst, name );
        return ip->make_SurdefList( lst );
    }
    return res;
}

Expr Scope::parse_ASSIGN( BinStreamReader bin ) {
    int name = read_nstring( bin );
    int flags = bin.read_positive_integer();
    Expr var = parse( bin.read_offset() );

    //
    if ( flags & IR_ASSIGN_TYPE )
        TODO;
        // Expr = apply( var, 0, 0, 0, 0, 0, class_scope ? APPLY_MODE_PARTIAL_INST : APPLY_MODE_STD, sf, off );

    //if ( ( flags & IR_ASSIGN_REF ) == 0 and var.referenced_more_than_one_time() )
    //    Expr = copy( var, sf, off );

    if ( flags & IR_ASSIGN_CONST ) {
        TODO;
        //        if ( ( var.flags & Var::WEAK_CONST ) == 0 and var.referenced_more_than_one_time() )
        //            disp_error( "Impossible to make this variable fully const (already referenced elsewhere)", sf, off );
        //        var.flags |= Var::WEAK_CONST;
        //        var.data->flags = PRef::CONST;
    }

    return ip->reg_var( name, var );
}

Expr Scope::parse_PUSH_IN_SCOPE( BinStreamReader bin ) {
    int flags = bin.read_positive_integer();
    Expr var = parse( bin.read_offset() );

    //
    if ( flags & IR_ASSIGN_TYPE )
        TODO;
        // Expr = apply( var, 0, 0, 0, 0, 0, class_scope ? APPLY_MODE_PARTIAL_INST : APPLY_MODE_STD, sf, off );

    if ( flags & IR_ASSIGN_CONST ) {
        TODO;
        //        if ( ( var.flags & Var::WEAK_CONST ) == 0 and var.referenced_more_than_one_time() )
        //            disp_error( "Impossible to make this variable fully const (already referenced elsewhere)", sf, off );
        //        var.flags |= Var::WEAK_CONST;
        //        var.data->flags = PRef::CONST;
    }

    if ( flags & IR_ASSIGN_STATIC )
        *static_vars << var;
    else
        local_vars << var;
    return var;
}

Expr Scope::parse_REASSIGN( BinStreamReader bin ) {
    Expr a = parse( bin.read_offset() );
    Expr b = parse( bin.read_offset() );
    return apply( get_attr( a, STRING_reassign_NUM ), 1, &b, 0, 0, 0, APPLY_MODE_STD );
}
Expr Scope::parse_GET_ATTR( BinStreamReader bin ) {
    Expr self = parse( bin.read_offset() );
    int attr = read_nstring( bin );
    if ( self.error())
        return ip->error_var();
    if ( Expr res = get_attr( self, attr ) )
        return res;
    return ip->ret_error( "No attribute " + ip->str_cor.str( attr ) );
}
Expr Scope::parse_GET_ATTR_PTR( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_GET_ATTR_ASK( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_GET_ATTR_PTR_ASK( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_GET_ATTR_PA( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_IF( BinStreamReader bin ) {
    Expr cond_if = parse( bin.read_offset() );
    if ( cond_if.error() )
        return cond_if;

    // bool conversion
    if ( cond_if->ptype() != ip->type_Bool ) {
        cond_if = apply( find_var( STRING_Bool_NUM ), 1, &cond_if );
        if ( cond_if.error() )
            return cond_if;
    }

    // simplified expression
    BoolOpSeq expr = *cond_if->get( cond );

    //
    const PI8 *ok = bin.read_offset();
    const PI8 *ko = bin.read_offset();

    // known value
    if ( expr.always( true ) ) {
        Scope if_scope( this, 0, "if_" + to_string( ok ) );
        return if_scope.parse( ok );
    }
    if ( expr.always( false ) ) {
        if ( ko ) {
            Scope if_scope( this, 0, "fi_" + to_string( ko ) );
            return if_scope.parse( ko );
        }
        return ip->void_var();
    }

    Expr res_ok;
    if ( ok ) {
        Scope if_scope( this, 0, "if_" + to_string( ok ) );
        if_scope.cond = expr;
        res_ok = if_scope.parse( ok );
    }

    Expr res_ko;
    if ( ko ) {
        Scope fi_scope( this, 0, "fi_" + to_string( ko ) );
        fi_scope.cond = not expr;
        res_ko = fi_scope.parse( ko );
    }

    return select( expr, res_ok, res_ko );
}

Expr Scope::parse_WHILE( BinStreamReader bin ) {
    TODO;
    return 0;
//    const PI8 *tok = bin.read_offset();

//    // stop condition
//    // created before nsv_date because we want to use set(...) to update it
//    // nevertheless, for each iteration where unknown are added, it has to start at true
//    Expr cont_var( &ip->type_Bool, cst( true ) );

//    IpSnapshot nsv;
//    ip->snapshots << &nsv;

//    // we repeat until there are no external modified values
//    int ne = ip->error_list.size();
//    std::map<Expr,Expr> unknowns;
//    for( unsigned old_nsv_size = 0, cpt = 0; ; old_nsv_size = nsv.changed.size(), ++cpt ) {
//        if ( cpt == 100 )
//            return ip->ret_error( "infinite loop during while parse" );

//        Scope wh_scope( this, "while_" + to_string( ip->off ) );
//        wh_scope.cont = cont_var;
//        wh_scope.parse( tok );

//        if ( ne != ip->error_list.size() )
//            return ip->error_var();

//        // if no new modified variables
//        if ( old_nsv_size == nsv.changed.size() )
//            break;

//        // replace each modified variable to a new unknown variables
//        // (to avoid simplifications during the next round)
//        for( auto &it : nsv.changed ) {
//            Expr unk = unknown_inst( it.second->size(), cpt );
//            unknowns[ it.first ] = unk;
//            const_cast<Expr &>( it.first )->_set_val( unk, unk->size(), Rese(), Expr() );
//        }
//        cont_var.set_val( Var( &ip->type_Bool, cst( true ) ), Rese(), Expr() );
//    }

//    // extract the cont_Expr from nsv (we don't want to output it twice)
//    nsv.changed.erase( cont_var.inst );

//    // corr table (output number -> input number)
//    // -> find if Unknown inst are used to compute the outputs
//    ++Inst::cur_op_id;
//    for( auto it : nsv.changed )
//        const_cast<Expr &>( it.first )->_get_val()->mark_children();
//    int cpt = 0;
//    Vec<int> corr;
//    Vec<int> inp_sizes;
//    for( auto it : nsv.changed ) {
//        if ( unknowns[ it.first ]->op_id == Inst::cur_op_id ) {
//            corr << cpt++;
//            inp_sizes << it.second->size();
//        } else
//            corr << -1;
//    }

//    // prepare a while inp (for initial values of variables modified in the loop)
//    Expr winp = while_inp( inp_sizes );

//    // set winp[...] as initial values of modified variables
//    cpt = 0;
//    for( auto &it : nsv.changed ) {
//        int num_inp = corr[ cpt++ ];
//        if ( num_inp >= 0 )
//            const_cast<Expr &>( it.first )->_set_val( get_nout( winp, num_inp ), Rese(), Expr() );
//        else
//            const_cast<Expr &>( it.first )->_set_val( it.second, Rese(), Expr() );
//    }

//    // and set cont_Expr to true (initial condition)
//    cont_var.set_val( Var( &ip->type_Bool, cst( true ) ), Rese(), Expr() );


//    // relaunch the while inst
//    Scope wh_scope( this, "while_" + to_string( ip->off ) );
//    wh_scope.cont = cont_var;
//    wh_scope.parse( tok );

//    // make the while instruction
//    Vec<Expr> out_exprs;
//    for( auto it : nsv.changed )
//        out_exprs << const_cast<Expr &>( it.first )->_get_val();
//    out_exprs << cont_var.get_val();
//    Expr wout = while_out( out_exprs );

//    cpt = 0;
//    Vec<Expr> inp_exprs;
//    for( auto it : nsv.changed )
//        if ( corr[ cpt++ ] >= 0 )
//            inp_exprs << simplified( it.second );
//    Expr wins = while_inst( inp_exprs, winp, wout, corr );

//    // replace changed variable by while_inst outputs
//    {
//        auto rs = raii_save( ip->snapshots );
//        ip->snapshots.resize( 0 );

//        int cpt = 0;
//        for( auto it : nsv.changed )
//            const_cast<Expr &>( it.first )->_set_val( get_nout( wins, cpt++ ) );
//    }

//    // break(s) to transmit ?
//    for( RemBreak rb : wh_scope.rem_breaks )
//        BREAK( rb.count, rb.cond );


//    ip->snapshots.pop_back();
//    return ip->void_var();
//}
//void Scope::BREAK( int n, Expr cond ) {
////    Expr c = cst( true );
////    for( Scope *s = this; s; s = s->caller ? s->caller : s->parent ) {
////        // found a for or a while ?
////        // (for_block = true for surrounding scope)
////        // (for_def_scope != 0 from ___bloc_exec call)
////        if ( s->cont.defined() or s->for_block ) {
////            if ( s->cont.defined() )
////                s->cont.set_val( Var( &ip->type_Bool, cst( false ) ) );
////            if ( n > 1 )
////                s->rem_breaks << RemBreak{ n - 1, c };
////            if ( s->cond.defined() )
////                s->cond = op( &ip->type_Bool, &ip->type_Bool, s->cond, &ip->type_Bool, op( &ip->type_Bool, &ip->type_Bool, c, Op_not_boolean() ), Op_and_boolean() );
////            else
////                s->cond = op( &ip->type_Bool, c, Op_not_boolean() );
////    ²        return;
////        }
////        if ( s->cond )
////            c = op( bt_Bool, bt_Bool, c, bt_Bool, s->cond, Op_and_boolean() );
////    }
//    return ip->disp_error( "nothing to break" );
}

Expr Scope::parse_BREAK( BinStreamReader bin ) {
//    int n = bin.read_positive_integer();
//    if ( not n )
//        n = 1;

//    // if we are breaking from a for, update n accordingly
//    for( Scope *s = this; s; s = s->parent ) {
//        // -> parsing from a while
//        if ( s->cont.defined() )
//            break;
//        // -> parsing from a for
//        if ( Scope *f = s->for_def_scope ) {
//            f = f->for_surrounding_scope; // f points to the surrounding for scope
//            for( Scope *s = this; s != f; s = s->parent ) {
//                if ( not s )
//                    return ip->ret_error( "Impossible to find the surrounding for scope" );
//                n += s->cont.defined() or s->for_block;
//            }
//            break;
//        }
//    }

//    //
//    BREAK( n );
    return ip->void_var();
}

Expr Scope::parse_CONTINUE( BinStreamReader bin ) {
    ip->disp_error( "continue", true );
    return ip->void_var();
}
Expr Scope::parse_FALSE( BinStreamReader bin ) {
    Bool val = false;
    return cst( ip->type_Bool, 1, &val );
}
Expr Scope::parse_TRUE( BinStreamReader bin ) {
    Bool val = true;
    return room( cst( ip->type_Bool, 1, &val ) );
}
Expr Scope::parse_VOID( BinStreamReader bin ) {
    return room( cst( ip->type_Void, 0, 0 ) );
}
Expr Scope::parse_SELF( BinStreamReader bin ) {
    if ( self )
        return self;
    return ip->ret_error( "not in an object" );
}
Expr Scope::parse_THIS( BinStreamReader bin ) {
    if ( not self )
        return ip->ret_error( "not in an object" );
    Vec<Expr> lt;
    lt << ip->make_type_var( self->type() );
    TODO;
    return 0;
    // return Var( ip->class_Ptr.type_for( lt ), self.ptr().get_val() );
}
Expr Scope::parse_FOR( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_IMPORT( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_NEW( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_LIST( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_LAMBDA( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_NULL_REF( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_AND( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_OR( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}

Expr Scope::parse_info( BinStreamReader bin ) {
    int n = bin.read_positive_integer();
    for( int i = 0; i < n; ++i )
        std::cout << ( i ? ", " : "" ) << parse( bin.read_offset() );
    std::cout << std::endl;
    return ip->void_var();
}
Expr Scope::parse_disp( BinStreamReader bin ) {
    int n = bin.read_positive_integer();
    for( int i = 0; i < n; ++i )
        std::cout << ( i ? ", " : "" ) << parse( bin.read_offset() )->get( cond );
    std::cout << std::endl;
    return ip->void_var();
}
Expr Scope::parse_rand( BinStreamReader bin ) {
    return symbol( ip->type_Bool, "rand()" );
}
Expr Scope::parse_syscall( BinStreamReader bin ) {
    int n = bin.read_positive_integer();
    Vec<Expr> inp( Rese(), n );
    for( int i = 0; i < n; ++i )
        inp << parse( bin.read_offset() )->get( cond );
    return syscall( inp );
}

#define CHECK_PRIM_ARGS( N ) \
    int n = bin.read_positive_integer(); \
    if ( n != N ) \
        return ip->ret_error( "Expecting " #N " operand" );

Expr Scope::parse_set_base_size_and_alig( BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 2 );
    Expr a = parse( bin.read_offset() );
    Expr b = parse( bin.read_offset() );
    if ( a.error() or b.error() )
        return ip->void_var();
    if ( a->get_val( ip->type_SI32, &base_size ) == false or b->get_val( ip->type_SI32, &base_alig ) == false ) {
        PRINT( a );
        PRINT( b );
        return ip->ret_error( "set_base_size_and_alig -> SI32/SI64 known values" );
    }
    return ip->void_var();
}
Expr Scope::parse_set_RawRef_dependancy( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_reassign_rec( BinStreamReader bin ) {
    int n = bin.read_positive_integer();
    if ( n == 1 ) {
        if ( not self )
            return ip->ret_error( "expecting a defined self" );
        self->set( parse( bin.read_offset() ), cond );
        return self;
    }
    if ( n != 2 )
        return ip->ret_error( "expecting 1 or 2 args" );
    Expr dst = parse( bin.read_offset() );
    Expr src = parse( bin.read_offset() );
    dst->set( src, cond );
    return dst;
}
Expr Scope::parse_assign_rec( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_set_ptr_val( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_select_SurdefList( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_ptr_size( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_ptr_alig( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_size_of( BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 1 );
    Expr T = parse( bin.read_offset() );
    TODO;
    return 0;
//    if ( Type *t = ip->type_from_type_var( T ) )
//        return Var( ip->type_ST, cst( ST( t->size() ) ) );
//    return ip->ret_error( "Expecting a type variable" );
}
Expr Scope::parse_alig_of( BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 1 );
    Expr T = parse( bin.read_offset() );
    TODO;
    return 0;
//    if ( Type *t = ip->type_from_type_var( T ) )
//        return Var( ip->type_ST, cst( ST( t->alig() ) ) );
//    return ip->ret_error( "Expecting a type variable" );
}
Expr Scope::parse_typeof( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_address( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_get_slice( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_pointed_value( BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 2 );
    Expr a = parse( bin.read_offset() );
    Expr T = parse( bin.read_offset() );
    TODO;
    return 0;
//    Type *type = ip->type_from_type_var( T );
//    if ( not type )
//        return ip->ret_error( "expecting a type Expr as second arg" );
//    return a.at( type );
}
Expr Scope::parse_pointer_on( BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 1 );
    Expr a = parse( bin.read_offset() );
    TODO;
    return 0;
//    return a.ptr();
}
Expr Scope::parse_block_exec( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_get_argc( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_get_argv( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_apply_LambdaFunc( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_inst_of( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}

template<class OP>
Expr Scope::parse_una( BinStreamReader bin, OP o ) {
    CHECK_PRIM_ARGS( 1 );
    Expr a = parse( bin.read_offset() );
    return op( a->get( cond ), o );
}

template<class OP>
Expr Scope::parse_bin( BinStreamReader bin, OP o ) {
    CHECK_PRIM_ARGS( 2 );
    Expr a = parse( bin.read_offset() );
    Expr b = parse( bin.read_offset() );
    return op( a->get( cond ), b->get( cond ), o );
}

#define DECL_IR_TOK( N ) Expr Scope::parse_##N( BinStreamReader bin ) { return parse_una( bin, Op_##N() ); }
#include "../Ir/Decl_UnaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( N ) Expr Scope::parse_##N( BinStreamReader bin ) { return parse_bin( bin, Op_##N() ); }
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK
