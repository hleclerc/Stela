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


#include "../Ast/Ast_Class.h"
#include "CstComputedSize.h"
#include "Class.h"
#include "Type.h"
#include "Room.h"
#include "Cst.h"
#include "Op.h"

Class::TrialClass::TrialClass( ParsingContext *caller, Class *orig ) : ns( 0, caller, "TrialClass_" + to_string( orig ) ), orig( orig ) {
    // ns.catched_vars = &orig->catched_vars;
}

Class::TrialClass::~TrialClass() {
}

Expr Class::TrialClass::call( int nu, Expr *vu, int nn, const String *names, Expr *vn, int pnu, Expr *pvu, int pnn, const String *pnames, Expr *pvn, int apply_mode, ParsingContext *caller, const Expr &cond, Expr self ) {
    Vec<Expr> args;
    for( String n : orig->ast_item->arguments )
        args << ns.get_var( n );

    Type *type = orig->type_for( args );
    ns.cond = and_boolean( ns.cond, cond );
    // ns.class_scope = type;

    // start with a unknown cst
    Expr ret;
    if ( type->size() < 0 ) {
        Expr func = type->find_static_attr( "size_init" );
        if ( func.error() )
            return caller->ret_error( "Impossible to find a static variable named 'size_init' in type (of variable size) to be instancied" );
        Expr val = caller->apply( func, nu, vu, nn, names, vn );
        ret = room( cst_computed_size( type, val ) );
    } else
        ret = room( cst( type, type->size(), 0, 0 ) );

    //
    if ( apply_mode == ParsingContext::APPLY_MODE_NEW )
        TODO;

    // call init
    if ( apply_mode == ParsingContext::APPLY_MODE_STD )
        ns.apply( ns.get_attr( ret, "init" ), nu, vu, nn, names, vn );

    return ret;
}

Class::Class( const Ast_Callable *ast_item ) : Callable( ast_item ) {
}

Callable::Trial *Class::test( int nu, Expr *vu, int nn, const String *names, Expr *vn, int pnu, Expr *pvu, int pnn, const String *pnames, Expr *pvn, ParsingContext *caller, Expr self ) {
    if ( ast_item->pertinence )
        TODO;
    TrialClass *res = new TrialClass( caller, this );

    // nb arguments
    if ( pnu + pnn < ast_item->min_nb_args() ) return res->wr( "no enough arguments" );
    if ( pnu + pnn > ast_item->max_nb_args() ) return res->wr( "To much arguments" );

    if ( ast_item->varargs ) {
        TODO;
    } else {
        // unnamed args
        for( int i = 0; i < pnu; ++i )
            res->ns.reg_var( ast_item->arguments[ i ], pvu[ i ] );
        // named args
        Vec<bool> used_arg( Size(), pnn, false );
        for( int i = pnu; i < ast_item->arguments.size(); ++i ) {
            String arg_name = ast_item->arguments[ i ];
            for( int n = 0; ; ++n ) {
                if ( n == pnn ) {
                    // unspecified arg
                    int j = i - ( ast_item->arguments.size() - ast_item->default_values.size() );
                    if ( j < 0 )
                        return res->wr( "unspecified mandatory argument" );
                    Expr v = ast_item->default_values[ j ]->parse_in( res->ns );
                    res->ns.reg_var( arg_name, v );
                    break;
                }
                if ( arg_name == pnames[ n ] ) {
                    res->ns.reg_var( arg_name, pvn[ n ] );
                    used_arg[ n ] = true;
                    break;
                }
            }
        }
        for( int n = 0; n < pnn; ++n ) {
            if ( not used_arg[ n ] ) {
                for( int m = 0; m < n; ++m )
                    if ( pnames[ n ] == pnames[ m ] )
                        caller->disp_error( "arg assigned twice", true );
                return res->wr( "name=... does not appear in def args" );
            }
        }
    }

    // condition
    if ( ast_item->condition ) {
        res->cond = ast_item->condition->parse_in( res->ns )->get( res->ns.cond );
        if ( res->cond->always( false ) )
            return res->wr( "condition = false" );
        if ( not res->cond->always( true ) ) {
            caller->disp_error( "class conditions must be known at compile time" );
            return res->wr( "condition not known at compile time" );
        }
    } else
        res->cond = true;

    return res;
}

static Expr const_or_copy( Expr &var ) {
    if ( var->cpt_use == 1 )
        var->flags |= Inst::CONST;
    if ( var->flags & Inst::CONST )
        return var;

    if ( var->ptype()->orig == ip->class_SurdefList ) {
        Expr i = ip->pc->apply( var, 0, 0, 0, 0, 0, ParsingContext::APPLY_MODE_PARTIAL_INST );
        return ip->pc->make_type_var( i->ptype() );
    }
    if ( var.inst->flags & Inst::CONST )
        return var;

    Expr res = ip->pc->copy( var );
    res.inst->flags |= Inst::CONST;
    return res;
}

static bool all_eq( Vec<Expr> &a, Vec<Expr> &b ) {
    if ( a.size() != b.size() )
        return false;
    for( int i = 0; i < a.size(); ++i )
        if ( a[ i ]->get() != b[ i ]->get() )
            return false;
    return true;
}

Type *Class::type_for( Vec<Expr> args ) {
    for( Type *t : types )
        if ( all_eq( t->parameters, args ) )
            return t;

    Type *res = new Type( this );
    for( int i = 0; i < args.size(); ++i )
        res->parameters << const_or_copy( args[ i ] );
    types << res;

    return res;
}


