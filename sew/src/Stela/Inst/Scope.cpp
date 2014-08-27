#include "../System/BinStreamReader.h"
#include "../System/UsualStrings.h"
#include "../System/ReadFile.h"
#include "../System/RaiiSave.h"
#include "../System/AutoPtr.h"
#include "../Ir/CallableFlags.h"
#include "../Ir/AssignFlags.h"
#include "../Ir/Numbers.h"
#include "../Ast/IrWriter.h"
#include "../Ast/Ast.h"
#include "../Met/Lexer.h"
#include "UnknownInst.h"
#include "Sourcefile.h"
#include "IpSnapshot.h"
#include <algorithm>
#include "ReplBits.h"
#include "Syscall.h"
#include "GetNout.h"
#include "Symbol.h"
#include "Select.h"
#include "While.h"
#include "Scope.h"
#include "Slice.h"
#include "Class.h"
#include "Room.h"
#include "Conv.h"
#include "Type.h"
#include "Def.h"
#include "Cst.h"
#include "Op.h"
#include "Ip.h"

Scope::Scope( Scope *parent, Scope *caller, String name, Ip *lip ) :
    parent( parent ), caller( caller ), cond( True() ), creation_date( IpSnapshot::cur_date ) {
    if ( parent ) {
        path = parent->path + "/";
        self = parent->self;
    }
    path += name;

    sf = caller ? caller->sf : ( parent ? parent->sf : 0 );

    do_not_execute_anything = false;
    static_vars   = ( lip ? lip : ip )->get_static_vars( path );
    class_scope   = 0;
    method        = false;
    disp_tok      = false;
    cont          = 0;
    for_block     = 0;
    catched_vars  = 0;
    base_size     = 0;
    base_alig     = 1;

    if ( caller )
        cond = caller->cond;
    else if ( parent )
        cond = parent->cond;

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

    // -> Ast
    AutoPtr<Ast> a = make_ast( ip->error_list, l.root(), true );
    if ( ip->error_list )
        return ip->error_var();

    // Ir -> fill sf->tok_data
    IrWriter t( a.ptr() );
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
    auto oscoper = raii_save( ip->cur_scope, this );
    auto oreason = raii_save( reason, nreason );
    auto osf     = raii_save( sf    , nsf );
    auto ooff    = raii_save( off );
    return _parse( ntok );
}

Expr Scope::_parse( const PI8 *tok ) {
    if ( tok == 0 or do_not_execute_anything )
        return ip->error_var();

    BinStreamReader bin( tok );
    PI8 tva = bin.get<PI8>(); ///< token type
    off = bin.read_positive_integer(); ///< offset in sourcefile

    switch ( tva ) {
        #define DECL_IR_TOK( N ) case IR_TOK_##N: if ( disp_tok ) std::cout << #N "\n"; return parse_##N( bin );
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
        res = parse( sf, tok, "block" );
    return res;
}

Expr Scope::parse_CALLABLE( BinStreamReader bin, Type *base_type ) {
    int name = read_nstring( bin );
    if ( disp_tok )
        std::cout << "name=" << ip->str_cor.str( name ) << std::endl;

    // supporting variable
    Callable *c = 0;
    if ( base_type == ip->type_Class ) {
        switch( name ) {
        #define DECL_BT( T ) case STRING_##T##_NUM: c = ip->class_##T; break;
        #include "DeclBaseClass.h"
        #include "DeclParmClass.h"
        #undef DECL_BT
        default: break;
        }
    }
    if ( not c ) {
        if      ( base_type == ip->type_Def   ) c = new Def;
        else if ( base_type == ip->type_Class ) c = new Class;
        else ERROR( "..." );
    }

    // fill in (read bin data)
    c->name = name;
    c->off  = off;
    c->sf   = sf;
    c->read_bin( this, bin );

    // get catched vars
    for( int n : c->pot_catched_vars )
        if ( Expr v = find_var( n, true ) )
            c->catched_vars.add( n, v );

    // class Def (or Class)
    //   cpp_inst_ptr ~= SI64 (inst in C++ of Def or Class)
    SI64 ptr = SI64( ST( c ) );
    Expr val = cst( base_type, 64, &ptr );

    // output var
    Expr res = room( val, Inst::SURDEF | Inst::CONST );
    c->var = res.inst;

    return reg_var( name, res );
}

Expr Scope::parse_DEF( BinStreamReader bin ) {
    return parse_CALLABLE( bin, ip->type_Def );
}

Expr Scope::parse_CLASS( BinStreamReader bin ) {
    return parse_CALLABLE( bin, ip->type_Class );
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
        // Callable * list
        Vec<Callable *> ci;
        Vec<Expr> surdefs = ip->get_args_in_varargs( slice( ip->type_from_type_var( f_type->parameters[ 0 ] ), f->get(), 0 )->get( cond ) );
        for( Expr surdef : surdefs ) {
            SI64 cptr_val;
            Expr cptr = rcast( ip->type_SI64, surdef->get( cond ) );
            if ( not cptr->get_val( ip->type_SI64, &cptr_val ) )
                return ip->ret_error( "exp. cst" );
            ci << reinterpret_cast<Callable *>( ST( cptr_val ) );
        }

        // parm
        Vec<int> pn_names;
        Vec<Expr> pu_args, pn_args;
        Type *parm_type = ip->type_from_type_var( f_type->parameters[ 1 ] );
        if ( parm_type != ip->type_Void ) {
            Expr parm_val = slice( parm_type, f->get( cond ), 1 * ip->ptr_size )->get( cond );
            Type *pt = parm_val->type();
            if ( pt->orig != ip->class_VarargsItemBeg and pt->orig != ip->class_VarargsItemEnd ) {
                return ip->ret_error( "expecting a vararg type (or void) as third arg of a callable type" );
            }

            int o = 0;
            //Expr vp = ( f.ptr() + 1 * ip->type_ST->size() ).at( ip->type_ST ); // pointer on varargs data
            while ( pt->orig != ip->class_VarargsItemEnd ) {
                //Expr p_arg = ( vp + o * ip->type_ST->size() ).at( ip->type_ST );

                SI32 tn;
                if ( not pt->parameters[ 1 ]->get( cond )->get_val( ip->type_SI32, &tn ) )
                    return ip->ret_error( "expecting a known SI32 as second arg of a varargs" );
                if ( tn >= 0 ) {
                    pn_args << slice( ip->type_from_type_var( pt->parameters[ 0 ] ), parm_val, o * ip->ptr_size );
                    pn_names << tn;
                } else {
                    pu_args << slice( ip->type_from_type_var( pt->parameters[ 0 ] ), parm_val, o * ip->ptr_size );
                }

                // iteration
                ++o;
                pt = ip->type_from_type_var( pt->parameters[ 2 ] );
                if ( pt->orig != ip->class_VarargsItemBeg and pt->orig != ip->class_VarargsItemEnd )
                    return ip->ret_error( "expecting a vararg type (or void) as third arg of a varargs" );
            }
        }
        int pnu = pu_args.size(), pnn = pn_args.size();

        // self
        Type *self_type = ip->type_from_type_var( f_type->parameters[ 2 ] );
        Expr self_ptr;
        if ( self_type != ip->type_Void )
            self_ptr = slice( self_type, f->get( cond ), ip->ptr_size * ( parm_type != ip->type_Void ) + ip->ptr_size );

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

            trials[ i ] = ci[ i ]->test( nu, u_args, nn, n_name, n_args, pnu, pu_args.ptr(), pnn, pn_names.ptr(), pn_args.ptr(), this, self_ptr );

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

        BoolOpSeq cond; // additional cond
        Vec<std::pair<BoolOpSeq,Expr> > res;
        for( int i = 0; i < nb_surdefs; ++i ) {
            if ( trials[ i ]->ok() ) {
                BoolOpSeq loc_cond = cond and trials[ i ]->cond;
                Expr loc = trials[ i ]->call( nu, u_args, nn, n_name, n_args, pnu, pu_args.ptr(), pnn, pn_names.ptr(), pn_args.ptr(), am, this, loc_cond, self_ptr );
                res << std::make_pair( loc_cond, loc );

                if ( trials[ i ]->cond.always( true ) )
                    break;
                cond = cond and not trials[ i ]->cond;
            }
        }
        Expr res_expr = res.back().second;
        for( int i = res.size() - 2; i >= 0; --i )
            res_expr = select( res[ i ].first, res[ i ].second, res_expr );
        return res_expr;
    }

    //
    if ( f_type == ip->type_Type ) {
        Expr res = room( cst( ip->type_from_type_var( f ) ) );
        if ( am == APPLY_MODE_NEW )
            TODO;

        // call init
        if ( am != APPLY_MODE_PARTIAL_INST )
            apply( get_attr( res, STRING_init_NUM ), nu, u_args, nn, n_name, n_args, Scope::APPLY_MODE_STD );
        return res;
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
    // shortcut (for bootstraping)
    if ( var->type()->pod() )
        return room( var->get( cond ) );
    //
    Expr val = var->get( cond );
    Expr res = room( cst( val->type(), var->size() ) );
    apply( get_attr( res, STRING_init_NUM ), 1, &var );
    return res;
}

static void keep_only_method_surdef( Vec<Expr> &lst, const BoolOpSeq &cond ) {
    for( int i = 0; i < lst.size(); ++i ) {
        if ( lst[ i ]->ptype()->orig == ip->class_Def ) {
            SI64 p;
            if ( not rcast( ip->type_SI64, lst[ i ]->get( cond ) )->get_val( ip->type_SI64, &p ) )
                return ip->disp_error( "weird Def" );
            if ( reinterpret_cast<Def *>( ST( p ) )->self_as_arg() )
                continue;
        }
        lst.remove( i );
    }
}

Expr Scope::get_first_attr( Expr self, int name ) {
    if ( self.error() )
        return (Inst *)0;

    Type *type = self->ptype(); type->parse();
    for( Type::Attr &at : type->attributes )
        if ( at.name == name )
            return at.off >= 0 ? rcast( at.val->type(), add( self, at.off ) ) : at.val;

    return (Inst *)0;
}

void Scope::get_attr_clist( Vec<Expr> &lst, Expr self, int name ) {
    if ( self.error() )
        return;

    Type *type = self->ptype(); type->parse();
    for( Type::Attr &at : type->attributes )
        if ( at.name == name and ( at.val->flags & Inst::SURDEF ) )
            lst << at.val;
}

Expr Scope::get_attr( Expr self, int name ) {
    if ( self.error() )
        return ip->error_var();
    if ( Expr res = get_first_attr( self, name ) )
        if ( not ( res->flags & Inst::SURDEF ) )
            return res;

    // not found or surdef ? -> search in global scope for def ...( self, ... )
    Vec<Expr> lst;
    find_var_clist( lst, name );
    keep_only_method_surdef( lst, cond );
    get_attr_clist( lst, self, name );
    if ( lst.size() == 0 )
        return ip->ret_error( "no attr '" + ip->str_cor.str( name ) + "' in object of type '" + to_string( *self->type() ) + "' (or surdef with self as arg in parent scopes)" );
    return ip->make_SurdefList( lst, self );
}

Expr Scope::parse_APPLY( BinStreamReader bin ) {
    Expr f = parse( sf, bin.read_offset(), "lhs" );

    int nu = bin.read_positive_integer();
    Expr u_args[ nu ];
    for( int i = 0; i < nu; ++i )
        u_args[ i ] = parse( sf, bin.read_offset(), "arg" );

    int nn = bin.read_positive_integer();
    int n_name[ nn ];
    Expr n_args[ nn ];
    for( int i = 0; i < nn; ++i ) {
        n_name[ i ] = read_nstring( bin );
        n_args[ i ] = parse( sf, bin.read_offset(), "arg" );
    }

    return apply( f, nu, u_args, nn, n_name, n_args, APPLY_MODE_STD );
}

Expr Scope::parse_SELECT( BinStreamReader bin ) {
    Expr f = parse( sf, bin.read_offset(), "rhs" );

    int nu = bin.read_positive_integer();
    Expr u_args[ nu ];
    for( int i = 0; i < nu; ++i )
        u_args[ i ] = parse( sf, bin.read_offset(), "arg" );

    int nn = bin.read_positive_integer();
    int n_name[ nn ];
    Expr n_args[ nn ];
    for( int i = 0; i < nn; ++i ) {
        n_name[ i ] = read_nstring(  bin );
        n_args[ i ] = parse( sf, bin.read_offset(), "arg" );
    }

    // shortcut for Callable
    Type *f_type = f->ptype();
    if ( f_type->orig == ip->class_SurdefList ) {
        // check
        Type *parm_type = ip->type_from_type_var( f_type->parameters[ 1 ] );
        if ( parm_type != ip->type_Void )
            TODO;

        Expr f_val = f->get( cond );
        Type *tp_0 = ip->type_from_type_var( f_type->parameters[ 0 ] );
        Type *tp_2 = ip->type_from_type_var( f_type->parameters[ 2 ] );

        // varargs format
        Vec<int> n;
        Vec<Expr> v;
        for( int i = 0; i < nu; ++i )
            v << u_args[ i ];

        for( int i = 0; i < nn; ++i ) {
            n << n_name[ i ];
            v << n_args[ i ];
        }

        Expr varg = ip->make_Varargs( v, n );

        Vec<Expr> lt;
        lt << f_type->parameters[ 0 ]; // Ptr[VarargItem[...]]
        lt << ip->make_type_var( varg->type() );
        lt << f_type->parameters[ 2 ]; // self ptr type
        Type *res_type = ip->class_SurdefList->type_for( lt );
        res_type->_len = 2 * ip->ptr_size + ( f_type->parameters[ 2 ] != ip->type_Void ) * ip->ptr_size;
        res_type->_ali = 32;

        Expr res = cst( res_type );
        res = repl_bits( res, 0 * ip->ptr_size, slice( tp_0, f_val, 0 ) );
        res = repl_bits( res, 1 * ip->ptr_size, varg );
        if ( f_type->parameters[ 2 ] != ip->type_Void )
            res = repl_bits( res, 2 * ip->ptr_size, slice( tp_2, f_val, 1 * ip->ptr_size ) );
        return room( res );
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
    switch ( ip->ptr_size ) {
    case 32: return parse_SI32( bin );
    case 64: return parse_SI64( bin );
    }
    return ip->ret_error( "wrong ip->ptr_size" );
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

Expr Scope::find_first_var( int name, bool exclude_main_scope ) {
    for( Scope *s = this; s; s = s->parent ) {
        if ( not s->parent ) { // we're in the main scope
            if ( self )
                if ( Expr res = s->get_first_attr( self, name ) )
                    return res;
            if ( exclude_main_scope )
                break;
        }
        // local
        if ( Expr res = s->local_vars.get( name ) )
            return res;
        // static
        if ( Expr res = s->static_vars->get( name ) )
            return res;
        // catched_vars
        if ( s->catched_vars )
            if ( Expr res = s->catched_vars->get( name ) )
                return res;
    }
    return (Inst *)0;
}

void Scope::find_var_clist( Vec<Expr> &lst, int name ) {
    for( Scope *s = this; s; s = s->parent ) {
        if ( not s->parent )
            if ( self )
                s->get_attr_clist( lst, self, name );
        // local
        s->local_vars.get( lst, name );
        // static
        s->static_vars->get( lst, name );
        // catched_vars
        if ( s->catched_vars )
            s->catched_vars->get( lst, name );
    }
}

Expr Scope::find_var( int name, bool exclude_main_scope ) {
    Expr res = find_first_var( name, exclude_main_scope );
    if ( res and res->is_surdef() ) {
        Vec<Expr> lst;
        find_var_clist( lst, name );
        return ip->make_SurdefList( lst, self );
    }
    return res;
}

Expr Scope::parse_ASSIGN( BinStreamReader bin ) {
    int name = read_nstring( bin );
    int flags = bin.read_positive_integer();
    Expr var = parse( sf, bin.read_offset(), "rhs" );

    //
    if ( flags & IR_ASSIGN_TYPE )
        var = apply( var, 0, 0, 0, 0, 0, class_scope ? APPLY_MODE_PARTIAL_INST : APPLY_MODE_STD );

    if ( var->referenced_more_than_one_time() and not ( flags & IR_ASSIGN_REF ) )
        var = copy( var );

    if ( flags & IR_ASSIGN_CONST ) {
        TODO;
        //        if ( ( var.flags & Var::WEAK_CONST ) == 0 and var.referenced_more_than_one_time() )
        //            disp_error( "Impossible to make this variable fully const (already referenced elsewhere)", sf, off );
        //        var.flags |= Var::WEAK_CONST;
        //        var.data->flags = PRef::CONST;
    }

    return reg_var( name, var, flags & IR_ASSIGN_STATIC );
}

Expr Scope::reg_var( int name, Expr var, bool stat ) {
    NamedVarList &vars = stat ? *static_vars : local_vars;
    if ( vars.contains( name ) and not var->is_surdef() ) {
        TODO;
        return ip->ret_error( "There is already a Expr named '" + ip->str_cor.str( name ) + "' in the current scope" );
    }
    return vars.add( name, var );
}


Expr Scope::parse_GET_ATTR( BinStreamReader bin ) {
    Expr self = parse( sf, bin.read_offset(), "rhs" );
    if ( self.error())
        return ip->error_var();

    int attr = read_nstring( bin );
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
    Expr cond_if = parse( sf, bin.read_offset(), "cond" );
    if ( cond_if.error() )
        return cond_if;

    // bool conversion
    if ( cond_if->ptype() != ip->type_Bool ) {
        cond_if = apply( find_var( STRING_Bool_NUM ), 1, &cond_if );
        if ( cond_if.error() )
            return ip->error_var();
    }

    // simplified expression
    BoolOpSeq expr( *cond_if->get( cond ) );

    //
    const PI8 *ok = bin.read_offset();
    const PI8 *ko = bin.read_offset();

    // known value
    if ( expr.always( true ) ) {
        Scope if_scope( this, 0, "if_" + to_string( ok ) );
        return if_scope.parse( sf, ok, "if" );
    }
    if ( expr.always( false ) ) {
        if ( ko ) {
            Scope if_scope( this, 0, "fi_" + to_string( ko ) );
            return if_scope.parse( sf, ko, "fi" );
        }
        return ip->void_var();
    }

    Expr res_ok;
    if ( ok ) {
        Scope if_scope( this, 0, "if_" + to_string( ok ) );
        if_scope.cond = expr;
        res_ok = if_scope.parse( sf, ok, "if" );
    }

    Expr res_ko;
    if ( ko ) {
        Scope fi_scope( this, 0, "fi_" + to_string( ko ) );
        fi_scope.cond = not expr;
        res_ko = fi_scope.parse( sf, ko, "fi" );
    }

    return select( expr, res_ok, res_ko );
}

Expr Scope::parse_WHILE( BinStreamReader bin ) {
    const PI8 *tok = bin.read_offset();

    // watch modified variables
    IpSnapshot nsv, *old_nsv = ip->cur_ip_snapshot;
    ip->cur_ip_snapshot = &nsv;

    // we repeat until there are no external modified values
    int ne = ip->error_list.size();
    std::map<Expr,Expr> unknowns;
    for( unsigned old_nsv_size = 0, cpt = 0; ; old_nsv_size = nsv.rooms.size(), ++cpt ) {
        if ( cpt == 100 )
            return ip->ret_error( "infinite loop during while parse" );

        Scope wh_scope( this, 0, "while_" + to_string( tok ) );
        BoolOpSeq cont_var; wh_scope.cont = &cont_var;
        wh_scope.parse( sf, tok, "while" );

        if ( ne != ip->error_list.size() )
            return ip->error_var();

        // if no new modified variables
        if ( old_nsv_size == nsv.rooms.size() )
            break;

        // replace each modified variable to a new unknown variables
        // (to avoid simplifications during the next round)
        for( std::pair<Inst *const,Expr> &it : nsv.rooms ) {
            Expr unk = unknown_inst( it.second->type(), cpt );
            unknowns[ it.first ] = unk;
            const_cast<Inst *>( it.first )->set( unk, BoolOpSeq() );
        }

        // reset all modified conds
        for( std::pair<Scope * const,BoolOpSeq> &it : nsv.conds )
            it.first->cond = it.second;
    }

    // corr table (output number -> input number)
    // -> find if Unknown inst are used to compute the outputs
    ++Inst::cur_op_id;
    for( std::pair<Inst *const,Expr> &it : nsv.rooms )
        const_cast<Inst *>( it.first )->get( cond )->mark_children();
    int cpt = 0;
    Vec<int> corr;
    Vec<Type *> inp_types;
    for( std::pair<Inst *const,Expr> &it : nsv.rooms ) {
        if ( unknowns[ it.first ]->op_id == Inst::cur_op_id ) {
            corr << cpt++;
            inp_types << it.second->type();
        } else
            corr << -1;
    }

    // prepare a while inp (for initial values of variables modified in the loop)
    Expr winp = while_inp( inp_types );

    // set winp[...] as initial values of modified variables
    cpt = 0;
    for( std::pair<Inst *const,Expr> &it : nsv.rooms ) {
        int num_inp = corr[ cpt++ ];
        if ( num_inp >= 0 )
            const_cast<Inst *>( it.first )->set( get_nout( winp, num_inp ), BoolOpSeq() );
        else
            const_cast<Inst *>( it.first )->set( it.second, BoolOpSeq() );
    }

    // relaunch the while inst
    Scope wh_scope( this, 0, "while_" + to_string( tok ) );
    BoolOpSeq cont_var; wh_scope.cont = &cont_var;
    wh_scope.parse( sf, tok, "while" );

    if ( cont_var.always( false) ) {
        ip->cur_ip_snapshot = old_nsv;
    } else {
        // make the while instruction
        Vec<Expr> out_exprs;
        Vec<Vec<Bool> > out_pos;
        for( std::pair<Inst *const,Expr> &it : nsv.rooms )
            out_exprs << const_cast<Inst *>( it.first )->get( cond );
        cont_var.get_out( out_exprs, out_pos );
        Expr wout = while_out( out_exprs, out_pos );

        cpt = 0;
        Vec<Expr> inp_exprs;
        for( std::pair<Inst *const,Expr> &it : nsv.rooms )
            if ( corr[ cpt++ ] >= 0 )
                inp_exprs << it.second->simplified( cond );
        Expr wins = while_inst( inp_exprs, winp, wout, corr );

        ip->cur_ip_snapshot = old_nsv;

        // replace changed variable by while_inst outputs
        cpt = 0;
        for( std::pair<Inst *const,Expr> &it : nsv.rooms )
            const_cast<Inst *>( it.first )->set( get_nout( wins, cpt++ ), cond );
    }

    // break(s) to transmit ?
    for( RemBreak rb : wh_scope.rem_breaks )
        BREAK( rb.count, rb.cond );

    return ip->void_var();
}

void Scope::BREAK( int n, BoolOpSeq cond ) {
    for( Scope *s = this; s; s = s->caller ? s->caller : s->parent ) {
        s->cond = s->cond and not cond;
        // found a for or a while ?
        if ( s->cont ) {
            *s->cont = *s->cont and not cond;
            if ( n > 1 )
                s->rem_breaks << RemBreak{ n - 1, cond };
            return;
        }
    }
    return ip->disp_error( "nothing to break" );
}

Expr Scope::parse_BREAK( BinStreamReader bin ) {
    int n = bin.read_positive_integer(), r = 0;

    // if we are breaking from a for, update n accordingly
    for( Scope *s = this; n and s; ) {
        // break from a for block
        if ( Scope *f = s->for_block ) {
            --n;
            for( Scope *s = this; s != f; s = s->caller ? s->caller : s->parent ) {
                if ( not s )
                    return ip->ret_error( "Impossible to find the surrounding for scope" );
                r += bool( s->cont );
            }
            s = f;
        } else {
            // break from a while
            if ( s->cont ) {
                --n;
                ++r;
            }
            //
            s = s->caller ? s->caller : s->parent;
        }
    }

    //
    BREAK( r, cond );
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
    //if ( self )
    //    return self;
    ERROR( "desactivated" );
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
    // names
    int nn = bin.read_positive_integer();
    int names[ nn ];
    for( int i = 0; i < nn; ++i )
        names[ i ] = read_nstring( bin );

    // objects
    int nc = bin.read_positive_integer();
    Expr vals[ nc ];
    for( int i = 0; i < nc; ++i )
        vals[ i ] = parse( sf, bin.read_offset(), "for rhs" );

    // catched variables
    Vec<int> catched_names;
    Vec<Expr> catched_values;
    int nb = bin.read_positive_integer();
    for( int i = 0; i < nb; ++i ) {
        int name = read_nstring( bin );
        if ( Expr e = find_var( name, true ) ) {
            catched_values << e;
            catched_names << name;
        }
    }

    const PI8 *tok = bin.read_offset();

    // block expr
    Vec<Expr> expr_names;
    for( int i = 0; i < nn; ++i )
        expr_names << names[ i ];

    Expr cv = ip->make_Varargs( catched_values, catched_names );

    Vec<Expr> lt;
    lt << room( ST( sf  ) );
    lt << room( ST( tok ) );
    lt << ip->make_type_var( cv->type() );
    lt << ip->make_Varargs( expr_names );

    Type *block_type = ip->class_Block->type_for( lt );
    Expr block = cst( block_type );
    block = repl_bits( block, 0, cv );
    block = repl_bits( block, cv->size(), SI64( ST( this ) ) );
    block = room( block );

    // exec
    if ( nc != 1 )
        TODO;
    return apply( get_attr( vals[ 0 ], STRING___for___NUM ), 1, &block );
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
Expr Scope::parse_AND( BinStreamReader bin ) {
    Expr a = parse( sf, bin.read_offset(), "arg" );
    if ( a.error() )
        return ip->error_var();

    // bool conversion
    Expr bool_a = a;
    if ( a->ptype() != ip->type_Bool ) {
        bool_a = apply( find_var( STRING_Bool_NUM ), 1, &a );
        if ( bool_a.error() )
            return ip->error_var();
    }

    // simplified expression
    BoolOpSeq expr( *bool_a->get( cond ) );

    // known value
    if ( expr.always( true ) )
        return parse( sf, bin.read_offset(), "2nd arg of and" );
    if ( expr.always( false ) )
        return a;

    // else
    const PI8 *tok_b = bin.read_offset();

    Scope if_scope( this, 0, "and_" + to_string( tok_b ) );
    if_scope.cond = expr;
    Expr b = if_scope.parse( sf, tok_b, "2nd arg of and" );

    return select( expr, b, a );
}
Expr Scope::parse_OR( BinStreamReader bin ) {
    Expr a = parse( sf, bin.read_offset(), "arg" );
    if ( a.error() )
        return ip->error_var();

    // bool conversion
    Expr bool_a = a;
    if ( a->ptype() != ip->type_Bool ) {
        bool_a = apply( find_var( STRING_Bool_NUM ), 1, &a );
        if ( bool_a.error() )
            return ip->error_var();
    }

    // simplified expression
    BoolOpSeq expr( *bool_a->get( cond ) );

    // known value
    if ( expr.always( true ) )
        return a;
    if ( expr.always( false ) )
        return parse( sf, bin.read_offset(), "2nd arg of and" );

    // else
    const PI8 *tok_b = bin.read_offset();

    Scope if_scope( this, 0, "or_" + to_string( tok_b ) );
    if_scope.cond = not expr;
    Expr b = if_scope.parse( sf, tok_b, "2nd arg of and" );

    return select( expr, a, b );
}

Expr Scope::parse_info( BinStreamReader bin ) {
    int n = bin.read_positive_integer();
    for( int i = 0; i < n; ++i )
        std::cout << ( i ? ", " : "" ) << parse( sf, bin.read_offset(), "info" );
    std::cout << std::endl;
    return ip->void_var();
}
Expr Scope::parse_disp( BinStreamReader bin ) {
    int n = bin.read_positive_integer();
    for( int i = 0; i < n; ++i ) {
        Expr r = parse( sf, bin.read_offset(), "disp" )->get( cond );
        std::cout << ( i ? ", " : "" ) << *r->type() << "{" << r << "}";
    }
    std::cout << std::endl;
    return ip->void_var();
}

#define CHECK_PRIM_ARGS( N ) \
    int n = bin.read_positive_integer(); \
    if ( n != N ) \
        return ip->ret_error( "Expecting " #N " operand" );

Expr Scope::parse_rand( BinStreamReader bin ) {
    static int num = 0;
    return room( symbol( ip->type_Bool, "rand(/*" + to_string( num++ ) + "*/)" ) );
}
Expr Scope::parse_syscall( BinStreamReader bin ) {
    int n = bin.read_positive_integer();
    Vec<Expr> inp( Rese(), n );
    for( int i = 0; i < n; ++i )
        inp << parse( sf, bin.read_offset(), "arg" )->get( cond );
    return room( syscall( inp, cond ) );
}

Expr Scope::parse_set_base_size_and_alig( BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 2 );
    Expr a = parse( sf, bin.read_offset(), "bs" )->get( cond );
    Expr b = parse( sf, bin.read_offset(), "ba" )->get( cond );
    if ( a.error() or b.error() )
        return ip->void_var();
    if ( a->get_val( ip->type_SI32, &base_size ) == false or b->get_val( ip->type_SI32, &base_alig ) == false )
        return ip->ret_error( "set_base_size_and_alig -> SI32/SI64 known values" );
    return ip->void_var();
}
Expr Scope::parse_set_RawRef_dependancy( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_reassign_rec( BinStreamReader bin ) {
    int n = bin.read_positive_integer();
    if ( n == 1 )
        return ip->ret_error( "2 args" );
    if ( n != 2 )
        return ip->ret_error( "expecting 1 or 2 args" );
    Expr dst = parse( sf, bin.read_offset(), "rhs" );
    Expr src = parse( sf, bin.read_offset(), "lhs" );
    Type *dt = dst->ptype();
    Type *st = src->ptype();
    if ( dt != st )
        dst->set( conv( dt, src->get( cond ) ), cond );
    else
        dst->set( src->get( cond ), cond );
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
    Expr T = parse( sf, bin.read_offset(), "sizeof" );
    Expr p = apply( T, 0, 0, 0, 0, 0, APPLY_MODE_PARTIAL_INST );
    return room( ST( p->ptype()->size() ) );
}
Expr Scope::parse_alig_of( BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 1 );
    Expr T = parse( sf, bin.read_offset(), "aligof" );
    Expr p = apply( T, 0, 0, 0, 0, 0, APPLY_MODE_PARTIAL_INST );
    return room( ST( p->ptype()->alig() ) );
}
Expr Scope::parse_typeof( BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 1 );
    Expr a = parse( sf, bin.read_offset(), "typeof" );
    return ip->make_type_var( a->get( cond )->type() );
}
Expr Scope::parse_address( BinStreamReader bin ) {
    TODO;
    return ip->error_var();
}
Expr Scope::parse_get_slice( BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 3 );
    Expr a = parse( sf, bin.read_offset(), "a" );
    Expr T = parse( sf, bin.read_offset(), "T" );
    Expr o = parse( sf, bin.read_offset(), "o" )->get( cond );
    Type *t = ip->type_from_type_var( T );
    return rcast( ip->ptr_for( t ), add( a, o ) );
}
Expr Scope::parse_pointed_value( BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 2 );
    Expr a = parse( sf, bin.read_offset(), "a" );
    Expr T = parse( sf, bin.read_offset(), "T" );
    TODO;
    return 0;
//    Type *type = ip->type_from_type_var( T );
//    if ( not type )
//        return ip->ret_error( "expecting a type Expr as second arg" );
//    return a.at( type );
}
Expr Scope::parse_pointer_on( BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 1 );
    return room( parse( sf, bin.read_offset(), "a" ) );
}
Expr Scope::parse_block_exec( BinStreamReader bin ) {
    CHECK_PRIM_ARGS( 2 );
    Expr blk = parse( sf, bin.read_offset(), "blk" )->get( cond );
    Expr val = parse( sf, bin.read_offset(), "val" );
    Type *blkt = blk->type();
    SourceFile *nsf; if ( not blkt->parameters[ 0 ]->get()->get_val( ip->type_ST, &nsf ) ) return ip->ret_error( "expecting a ST" );
    const PI8  *tok; if ( not blkt->parameters[ 1 ]->get()->get_val( ip->type_ST, &tok ) ) return ip->ret_error( "expecting a ST" );

    // catched
    Vec<int> catched_names;
    Vec<Expr> catched_values;
    ip->get_args_in_varargs( catched_values, catched_names,
                             slice( ip->type_from_type_var( blkt->parameters[ 2 ] ), blk, 0 )->get( cond ) );
    NamedVarList catched_vars;
    for( int i = 0; i < catched_names.size(); ++i )
        catched_vars.add( catched_names[ i ], catched_values[ i ] );

    // orig scope
    SI64 p_orig_scope;
    if ( not slice( ip->type_SI64, blk, ip->ptr_size )->get_val( ip->type_SI64, &p_orig_scope ) )
        return ip->ret_error( "expecting a known SI64" );

    //
    Vec<Expr> expr_names = ip->get_args_in_varargs( blkt->parameters[ 3 ]->get( cond ) );
    if ( expr_names.size() != 1 )
        return ip->ret_error( "todo: nb args != 1" );
    int name;
    if ( not expr_names[ 0 ]->get_val( ip->type_SI32, &name ) )
        return ip->ret_error( "expecting a known value" );

    // catched +
    Scope ns( &ip->main_scope, this, "block_exec" );
    ns.catched_vars = &catched_vars;
    ns.local_vars.add( name, val );
    ns.for_block = reinterpret_cast<Scope *>( ST( p_orig_scope ) );

    return ns.parse( nsf, tok, "block_exec" );
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
    Expr a = parse( sf, bin.read_offset(), "rhs" );
    return room( op( a->get( cond ), o ) );
}

template<class OP>
Expr Scope::parse_bin( BinStreamReader bin, OP o ) {
    CHECK_PRIM_ARGS( 2 );
    Expr a = parse( sf, bin.read_offset(), "rhs" );
    Expr b = parse( sf, bin.read_offset(), "lhs" );
    return room( op( a->get( cond ), b->get( cond ), o ) );
}

#define DECL_IR_TOK( N ) Expr Scope::parse_##N( BinStreamReader bin ) { return parse_una( bin, Op_##N() ); }
#include "../Ir/Decl_UnaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( N ) Expr Scope::parse_##N( BinStreamReader bin ) { return parse_bin( bin, Op_##N() ); }
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK
