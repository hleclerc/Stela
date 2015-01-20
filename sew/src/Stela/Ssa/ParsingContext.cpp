/****************************************************************************
**
** Copyright (C) 2014 SocaDB
**
** This file is part of the SocaDB toolkit/database.
**
** SocaDB is free software. You can redistribute this file and/or modify
** it under the terms of the Apache License, Version 2.0 (the "License").
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
** You should have received a copy of the Apache License, Version 2.0
** along with this program. If not, see
** <http://www.apache.org/licenses/LICENSE-2.0.html>.
**
**
** Commercial License Usage
**
** Alternatively, licensees holding valid commercial SocaDB licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and SocaDB.
**
**
****************************************************************************/

#include "../System/FileExists.h"
#include "../System/ReadFile.h"
#include "../Ast/Ast_Callable.h"
#include "../Ast/Ast_Lambda.h"
#include "../Met/Lexer.h"
#include "CstComputedSize.h"
#include "ParsingContext.h"
#include "IpSnapshot.h"
#include "SurdefList.h"
#include "Callable.h"
#include "Varargs.h"
#include "Select.h"
#include "Class.h"
#include "Rcast.h"
#include "Type.h"
#include "Room.h"
#include "Cst.h"
#include "Def.h"
#include "Op.h"
#include <algorithm>
#include <limits>

ParsingContext::ParsingContext( ParsingContext *parent, ParsingContext *caller, String add_scope_name ) : parent( parent ), caller( caller ) {
    if ( parent ) {
        scope_name = parent->scope_name;
        cond = parent->cond;
    } else {
        cond = true;
    }

    scope_name += to_string( add_scope_name.size() ) + "_" + add_scope_name;
    scope_type  = SCOPE_TYPE_STD;
    base_size   = 0;
    base_alig   = 1;
    current_src = 0;
    for_block   = 0;

    static std::map<String,Vec<NamedVar> > static_variables_map;
    static_variables = &static_variables_map[ scope_name ];
}

ParsingContext::~ParsingContext() {
    for( IpSnapshot *is = ip->ip_snapshot; is; is = is->prev )
        is->parsing_contexts.erase( this );
}

void ParsingContext::add_inc_path( String path ) {
    ip->include_paths.push_back_unique( path );
}

void ParsingContext::parse( String filename, String current_dir ) {
    int old_error_list_size = ip->error_list.size();

    // find the file
    filename = find_src( filename, current_dir );
    if ( filename.size() == 0 or ip->already_parsed.count( filename ) )
        return;
    ip->already_parsed.insert( filename );

    // -> source data
    ReadFile r( filename.c_str() );
    if ( not r )
        return disp_error( "Impossible to open " + filename );

    // -> lexical data
    Lexer lexer( ip->error_list );
    lexer.parse( r.data, filename.c_str() );
    if ( ip->error_list.size() != old_error_list_size )
        return;

    // -> parse Ast
    Past ast = make_ast( ip->error_list, lexer.root(), true );
    ast->parse_in( *this );
    ip->ast_lst << ast;
}

String ParsingContext::find_src( String filename, String current_dir ) const {
    // absolute path ?
    if ( filename[ 0 ] == '/' or filename[ 0 ] == '\\' )
        return file_exists( filename ) ? filename : String();

    // try with current_dir
    if ( current_dir.size() ) {
        String trial = current_dir + '/' + filename;
        if ( file_exists( trial ) )
            return trial;
    }

    // try with inc_paths
    for( String path : ip->include_paths ) {
        String trial = path + '/' + filename;
        if ( file_exists( trial ) )
            return trial;
    }

    // not found :(
    return String();
}

Expr ParsingContext::copy( Expr var ) {
    if ( var.error() )
        return var;
    // shortcut (for bootstraping)
    if ( var->ptype()->pod() )
        return room( var->get( cond ) );
    //
    Expr val = var->get( cond );
    Expr res = room( cst_computed_size( val->type(), var->size() ) );
    apply( get_attr( res, "init" ), 1, &var );
    return res;
}

Expr ParsingContext::reg_var( String name, Expr expr, bool stat, int _off, const String *_src ) {
    if ( scope_type == SCOPE_TYPE_MAIN ) {
        Vec<GlobalVariables::Variable,-1,1> &v = ip->main_scope[ name ];
        if ( v.size() and not expr->is_surdef() )
            disp_error( "Variable " + name + " is already defined" );
        v << GlobalVariables::Variable{ expr };
        return expr;
    }

    // std case
    NamedVar *nv = stat ? static_variables->push_back() : variables.push_back();
    nv->off  = _src ? _off : current_off;
    nv->src  = _src ? _src : current_src;
    nv->name = name;
    nv->expr = expr;
    return expr;
}

Expr ParsingContext::_find_first_var_with_name( String name ) {
    for( ParsingContext *c = this; c; c = c->parent ) {
        for( NamedVar &nv : c->variables )
            if ( nv.name == name )
                return nv.expr;
        for( NamedVar &nv : *c->static_variables )
            if ( nv.name == name )
                return nv.expr;
    }
    std::map<String,Vec<GlobalVariables::Variable,-1,1> >::const_iterator iter = ip->main_scope.find( name );
    if ( iter != ip->main_scope.end() )
        return iter->second[ 0 ].expr;
    return Expr();
}

void ParsingContext::_find_list_of_vars_with_name( Vec<Expr> &res, String name ) {
    for( ParsingContext *c = this; c; c = c->parent ) {
        for( NamedVar &nv : c->variables )
            if ( nv.name == name )
                res << nv.expr;
        for( NamedVar &nv : *c->static_variables )
            if ( nv.name == name )
                res << nv.expr;
    }
    std::map<String,Vec<GlobalVariables::Variable,-1,1> >::const_iterator iter = ip->main_scope.find( name );
    if ( iter != ip->main_scope.end() )
        for( const GlobalVariables::Variable &v : iter->second )
            res << v.expr;
}

Expr ParsingContext::get_var( String name, bool disp_err ) {
    Expr res = _find_first_var_with_name( name );
    if ( res and res->is_surdef() ) {
        Vec<Expr> lst;
        _find_list_of_vars_with_name( lst, name );
        return _make_surdef_list( lst );
    }
    if ( disp_err and not res )
        disp_error( "Impossible to find variable '" + name + "' from current scope" );
    return res;
}

Expr ParsingContext::_get_first_attr( Expr self, String name ) {
    if ( self.error() )
        return self;

    Type *type = self->ptype();
    type->parse();

    for( Type::Attr &at : type->attributes )
        if ( at.name == name )
            return type->attr_expr( self, at );

    return Expr();
}

void ParsingContext::_get_attr_clist( Vec<Expr> &lst, Expr self, String name ) {
    if ( self.error() )
        return;

    Type *type = self->ptype();
    type->parse();

    for( Type::Attr &at : type->attributes )
        if ( at.name == name and ( at.val->flags & Inst::SURDEF ) )
            lst << at.val;
}

void ParsingContext::_keep_only_method_surdefs( Vec<Expr> &lst ) {
    // keep only defs with self_as_arg
    for( int i = 0; i < lst.size(); ++i ) {
        if ( lst[ i ]->ptype()->orig == ip->class_Def ) {
            SI64 p;
            if ( not lst[ i ]->get( cond )->get_val( &p, 64 ) )
                return disp_error( "weird Def" );
            if ( reinterpret_cast<Def *>( ST( p ) )->ast_item->self_as_arg )
                continue;
        }
        lst.remove( i );
    }
}

Expr ParsingContext::get_attr( Expr self, String name ) {
    if ( self.error() )
        return self;
    if ( Expr res = _get_first_attr( self, name ) )
        if ( not ( res->flags & Inst::SURDEF ) )
            return res;

    // not found or surdef ? -> search in global scope for def ...( self, ... )
    Vec<Expr> lst;
    _find_list_of_vars_with_name( lst, name );
    _keep_only_method_surdefs( lst );
    _get_attr_clist( lst, self, name );
    if ( lst.size() == 0 )
        return ret_error( "no attr '" + name + "' in object of type '" + to_string( *self->ptype() ) + "' (or surdef with self as arg in parent scopes)" );
    return _make_surdef_list( lst, self );
}

Expr ParsingContext::apply( Expr f, int nu, Expr *u_args, int nn, const String *n_name, Expr *n_args, ApplyMode am, Varargs *va_size_init ) {
    if ( f.error() )
        return f;
    for( int i = 0; i < nu; ++i )
        if ( u_args[ i ].error() )
            return u_args[ i ];
    for( int i = 0; i < nn; ++i )
        if ( n_args[ i ].error() )
            return u_args[ i ];

    // SurdefList( ... )
    Type *f_type = f->ptype();
    if ( f_type->orig == ip->class_SurdefList ) {
        //        // Callable * list
        //        Vec<Callable *> ci;
        //        Vec<Expr> surdefs = ip->get_args_in_varargs( slice( ip->type_from_type_var( f_type->parameters[ 0 ] ), f->get(), 0 )->get( cond ) );
        //        for( Expr surdef : surdefs ) {
        //            SI64 cptr_val;
        //            Expr cptr = rcast( ip->type_SI64, surdef->get( cond ) );
        //            if ( not cptr->get_val( ip->type_SI64, &cptr_val ) )
        //                return ip->ret_error( "exp. cst" );
        //            ci << reinterpret_cast<Callable *>( ST( cptr_val ) );
        //        }

        SI64 sp;
        Expr rf = f->get( cond );
        if ( not rf or not rf->get_val( &sp, 64 ) )
            return ret_error( "expecting a cst" );
        SurdefList *s = reinterpret_cast<SurdefList *>( ST( sp ) );

        // Callable * list
        struct CallableAndPertinence {
            CallableAndPertinence( SI64 sp ) {
                c = reinterpret_cast<Callable *>( ST( sp ) );
                pertinence = c->ast_item->pertinence ? std::numeric_limits<double>::max() : c->ast_item->default_pertinence();
            }

            Callable *c;
            double    pertinence;
        };
        Vec<CallableAndPertinence> ci( Rese(), s->callables.size() );
        for( Expr e : s->callables ) {
            Expr re = e->get( cond );
            if ( not re->get_val( &sp, 64 ) )
                return ret_error( "expecting a cst for surdefs" );
            ci.push_back( sp );
        }

        // parm
        Vec<String> pn_names = s->parameters.names;
        Vec<Expr> pu_args = s->parameters.u_args(), pn_args = s->parameters.n_args();
        int pnu = pu_args.size(), pnn = pn_args.size();

        // self
        Expr self_ptr = s->self;
        //        Type *self_type = ip->type_from_type_var( f_type->parameters[ 2 ] );
        //        Expr self_ptr;
        //        if ( self_type != ip->type_Void )
        //            self_ptr = slice( self_type, f->get( cond ), ip->ptr_size * ( parm_type != ip->type_Void ) + ip->ptr_size );

        // computed pertinences
        int nb_surdefs = ci.size();
        AutoPtr<Callable::Trial> trials[ nb_surdefs ];
        for( int i = 0; i < nb_surdefs; ++i ) {
            if ( ci[ i ].c->ast_item->pertinence ) {
                trials[ i ] = ci[ i ].c->test( nu, u_args, nn, n_name, n_args, pnu, pu_args.ptr(), pnn, pn_names.ptr(), pn_args.ptr(), this, self_ptr );
                ci[ i ].pertinence = trials[ i ]->computed_pertinence;
            }
        }

        // sort by pertinence
        struct CmpCallableInfobyPertinence {
            bool operator()( const CallableAndPertinence &a, const CallableAndPertinence &b ) const {
                return a.pertinence > b.pertinence;
            }
        };
        std::sort( ci.begin(), ci.end(), CmpCallableInfobyPertinence() );

        // test items without computed pertinence
        int nb_ok = 0;
        double guaranted_pertinence = 0;
        bool has_guaranted_pertinence = false;
        for( int i = 0; i < nb_surdefs; ++i ) {
            if ( has_guaranted_pertinence and guaranted_pertinence > ci[ i ].pertinence ) {
                for( int j = i; j < nb_surdefs; ++j )
                    trials[ j ] = new Callable::Trial( "Already a more pertinent solution" );
                break;
            }

            if ( not ci[ i ].c->ast_item->pertinence ) // if test not done during the computed pertinence step
                trials[ i ] = ci[ i ].c->test( nu, u_args, nn, n_name, n_args, pnu, pu_args.ptr(), pnn, pn_names.ptr(), pn_args.ptr(), this, self_ptr );


            if ( trials[ i ]->ok() ) {
                if ( trials[ i ]->cond.error() )
                    return trials[ i ]->cond;
                if ( trials[ i ]->cond->always( true ) ) {
                    has_guaranted_pertinence = true;
                    guaranted_pertinence = ci[ i ].pertinence;
                }
                ++nb_ok;
            }
        }

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
            ErrorList::Error &err = error_msg( ss.str() );
            for( int i = 0; i < ci.size(); ++i )
                err.ap( ci[ i ].c->ast_item->_src->c_str(), ci[ i ].c->ast_item->_off, std::string( "possibility (" ) + trials[ i ]->reason + ")" );
            std::cerr << err;
            return Expr();
        }

        // valid surdefs, but only with runtime conditions
        if ( not has_guaranted_pertinence ) {
            std::ostringstream ss;
            ss << "There is no failback surdefinition (only runtime conditions)";
            ErrorList::Error &err = ip->pc->error_msg( ss.str() );
            for( int i = 0; i < nb_surdefs; ++i ) {
                if ( trials[ i ]->ok() )
                    err.ap( ci[ i ].c->ast_item->_src->c_str(), ci[ i ].c->ast_item->_off, "accepted" );
                else
                    err.ap( ci[ i ].c->ast_item->_src->c_str(), ci[ i ].c->ast_item->_off, std::string( "rejected (" ) + trials[ i ]->reason + ")" );
            }
            std::cerr << err;
            return Expr();
        }

        // ambiguous overload ?
        if ( nb_ok > 1 ) {
            double best_pertinence = - std::numeric_limits<double>::max();
            for( int i = 0; i < nb_surdefs; ++i )
                best_pertinence = std::max( best_pertinence, ci[ i ].pertinence );
            int nb_wp = 0;
            for( int i = 0; i < nb_surdefs; ++i )
                nb_wp += trials[ i ]->ok() and ci[ i ].pertinence == best_pertinence;
            if ( nb_wp > 1 ) {
                std::ostringstream ss;
                ss << "Ambiguous overload";
                ErrorList::Error &err = error_msg( ss.str() );
                for( int i = 0; i < nb_surdefs; ++i )
                    if ( trials[ i ]->ok() and ci[ i ].pertinence == best_pertinence )
                        err.ap( ci[ i ].c->ast_item->_src->c_str(), ci[ i ].c->ast_item->_off, "possibility" );
                std::cerr << err;
                return Expr();
            }
        }

        Expr cond( true ); // additional cond
        Vec<std::pair<Expr,Expr> > res; // cond -> result of apply
        for( int i = 0; i < nb_surdefs; ++i ) {
            if ( trials[ i ]->ok() ) {
                Expr loc_cond = and_boolean( cond, trials[ i ]->cond );
                Expr loc = trials[ i ]->call( nu, u_args, nn, n_name, n_args, pnu, pu_args.ptr(), pnn, pn_names.ptr(), pn_args.ptr(), am, this, loc_cond, self_ptr, va_size_init );
                res << std::make_pair( loc_cond, loc );

                if ( trials[ i ]->cond->always( true ) )
                    break;
                cond = and_boolean( cond, not_boolean( trials[ i ]->cond ) );
            }
        }
        Expr res_expr = res.back().second;
        for( int i = res.size() - 2; i >= 0; --i )
            res_expr = select( res[ i ].first, res[ i ].second, res_expr );
        return res_expr;
    }

    //
    if ( f_type == ip->type_Type ) {
        ParsingContext ns( this );
        Type *type = type_from_type_expr( f );
        return Class::call( ns, type, nu, u_args, nn, n_name, n_args, 0, 0, 0, 0, 0, am, this, cond, self, 0 );
    }

    // Lambda
    if ( f_type == ip->type_Lambda ) {
        SI64 pl;
        if ( not f->get( cond )->get_val( &pl, 64 ) )
            return ret_error( "expecting a known value", false, __FILE__ , __LINE__ );
        Ast_Lambda *al = reinterpret_cast<Ast_Lambda *>( ST( pl ) );
        if ( nn )
            return ret_error( "TODO: named arguments for lambda expressions" );
        if ( al->names.size() > nu )
            return ret_error( "Too much arguments for lambda expressions" );
        ParsingContext ns( 0, this, "Lambda_" + to_string( al ) );
        for( int i = 0; i < al->names.size(); ++i )
            ns.reg_var( al->names[ i ], u_args[ i ] );
        return al->body->parse_in( ns );
    }

    // f.apply ...
    //    Expr applier = get_attr( f, STRING_apply_NUM );
    //    if ( applier.error() )
    //        return ip->error_var();

    //    return apply( applier, nu, u_args, nn, n_name, n_args, am );
    disp_error( "call for type..." );
    PRINT( *f_type );
    PRINT( f );
    TODO;
    return Expr();
}

bool ParsingContext::always_equal( Expr a, Expr b ) {
    return a->get( cond ) == b->get( cond );
}

Type *ParsingContext::type_from_type_expr( Expr type_expr ) {
    ASSERT( type_expr->ptype() == ip->type_Type, "..." );
    SI64 p;
    if ( not type_expr->get( cond )->get_val( &p, 64 ) )
        return 0;
    return reinterpret_cast<Type *>( ST( p ) );
}

Type *ParsingContext::ptr_type_for( Type *ref ) {
    return ip->class_Ptr->type_for( type_expr( ref ) );
}

Expr ParsingContext::type_expr( Type *ref ) {
    SI64 ref_ptr = ST( ref );
    return room( cst( ip->type_Type, 64, &ref_ptr ) );
}


Expr ParsingContext::_make_surdef_list( const Vec<Expr> &lst, Expr self ) {
    SurdefList *res = new SurdefList;
    res->callables = lst;
    res->self      = self;
    SI64 val = ST( res );
    return room( cst( ip->type_SurdefList, 64, &val ) );
}

void ParsingContext::disp_error( String msg, bool warn, const char *file, int line ) {
    std::cerr << error_msg( msg, warn, file, line );
}

ErrorList::Error &ParsingContext::error_msg( String msg, bool warn, const char *file, int line ) {
    ErrorList::Error &res = ip->error_list.add( msg, warn );
    if ( file )
        res.caller_stack.push_back( line, file );
    for( ParsingContext *p = this; p; p = p->caller ? p->caller : p->parent )
        if ( p->current_src )
            res.ac( p->current_src->c_str(), p->current_off );
    return res;
}

Expr ParsingContext::ret_error( String msg, bool warn, const char *file, int line ) {
    disp_error( msg, warn, file, line );
    return error_var();
}

Expr ParsingContext::error_var() {
    return Expr();
}

void ParsingContext::BREAK( int n, Expr cond ) {
    for( ParsingContext *s = this; s; s = s->caller ? s->caller : s->parent ) {
        if ( IpSnapshot *is = ip->ip_snapshot )
            is->reg_parsing_context( s );
        s->cond = and_boolean( s->cond, not_boolean( cond ) );
        // found a for or a while ?
        if ( s->cont ) {
            s->cont = and_boolean( s->cont, not_boolean( cond ) );
            if ( n > 1 )
                s->rem_breaks << RemBreak{ n - 1, cond };
            return;
        }
    }

    return disp_error( "nothing to break" );
}

Expr ParsingContext::_make_varars( Vec<Expr> lst, const Vec<String> &names ) {
    Varargs *res = new Varargs;
    res->exprs = lst;
    res->names = names;
    SI64 val = ST( res );
    return room( cst( ip->type_Varargs, 64, &val ) );

}
