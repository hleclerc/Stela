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


#include "../Ast/Ast_Def.h"
#include "Class.h"
#include "Type.h"
#include "Inst.h"
#include "Def.h"
#include "Op.h"

Def::TrialDef::TrialDef( ParsingContext *caller, Def *orig ) : orig( orig ), ns( 0, caller, "TrialDef_" + to_string( orig ) ) {
    // ns.catched_vars = &orig->catched_vars;
}

Def::TrialDef::~TrialDef() {
}

Expr Def::TrialDef::call( int nu, Expr *vu, int nn, const String *names, Expr *vn, int pnu, Expr *pvu, int pnn, const String *pnames, Expr *pvn, int apply_mode, ParsingContext *caller, const Expr &cond, Expr self ) {
    if ( apply_mode != ParsingContext::APPLY_MODE_STD )
        TODO;

    //
    ns.cond = and_boolean( ns.cond, cond );

    // particular case
    if ( orig->ast_item->name == "init" ) {
        TODO;
        //        // init attributes
        //        Type *self_type = self->ptype();

        //        if ( self_type->orig->ancestors.size() )
        //            TODO;

        //        Vec<Bool> initialised_args( Size(), self_type->attributes.size(), false );
        //        for( AttrInit &d : orig->attr_init ) {
        //            for( int i = 0; ; ++i ) {
        //                if ( i == self_type->attributes.size() )
        //                    return ip->ret_error( "no attribute " + ip->str_cor.str( d.attr ) + " in class" );

        //                Type::Attr &a = self_type->attributes[ i ];
        //                if ( d.attr == a.name ) {
        //                    // parse args
        //                    Vec<Expr> args;
        //                    int nu = d.args.size() - d.names.size();
        //                    for( Code &c : d.args )
        //                        args << ns.parse( c.sf, c.tok, "starts_with" );

        //                    //
        //                    ns.apply( ns.get_attr( rcast( a.val->type(), add( self, a.off ) ), STRING_init_NUM ),
        //                              nu, args.ptr(), d.names.size(), d.names.ptr(), args.ptr() + nu );
        //                    initialised_args[ i ] = true;
        //                    break;
        //                }
        //            }
        //        }

        //        int cpt = 0;
        //        for( Type::Attr &a : self_type->attributes )
        //            if ( a.off >= 0 and not initialised_args[ cpt ] )
        //                ns.apply( ns.get_attr( rcast( a.val->type(), add( self, a.off ) ), STRING_init_NUM ), not ( a.val->flags & Inst::PART_INST ), &a.val );
        //            ++cpt;
    }

    // inline call
    return orig->ast_item->block->parse_in( ns );
}

Def::Def( const Ast_Callable *ast_item ) : Callable( ast_item ) {
}

Callable::Trial *Def::test( int nu, Expr *vu, int nn, const String *names, Expr *vn, int pnu, Expr *pvu, int pnn, const String *pnames, Expr *pvn, ParsingContext *caller, Expr self ) {
    if ( ast_item->pertinence )
        TODO;
    TrialDef *res = new TrialDef( caller, this );
    if ( self ) {
        res->ns.reg_var( "self", self );
        res->ns.self = self;
    }

    // nb arguments
    if ( pnu + pnn + nu + nn < ast_item->min_nb_args() ) return res->wr( "no enough arguments" );
    if ( pnu + pnn + nu + nn > ast_item->max_nb_args() ) return res->wr( "To much arguments" );

    if ( pnu + pnn )
        TODO;
    if ( ast_item->varargs ) {
        TODO; // var ordering

        //        Vec<Expr> v_args;
        //        Vec<int> v_names;
        //        for( int i = 0; i < nn; ++i ) {
        //            int o = arg_names.first_index_equal_to( names[ i ] );
        //            if ( o >= 0 ) {
        //                if ( arg_ok[ o ] )
        //                    ip->disp_error( "arg is already assigned", true );
        //                scope.local_vars << vn[ i ];
        //                arg_ok[ o ] = true;
        //            } else {
        //                v_args  << vn   [ i ];
        //                v_names << names[ i ];
        //            }
        //        }

        //        for( int i = 0; i < arg_names.size(); ++i ) {
        //            if ( arg_ok[ i ] )
        //                ip->disp_error( "arg is already assigned", true );
        //            scope.local_vars << vu[ i ];
        //            arg_ok[ i ] = true;
        //        }
        //        for( int i = arg_names.size(); i < nu; ++i )
        //            v_args << vu[ i ];


        //        Expr varargs = ip->make_Varargs( v_args, v_names );
        //        scope.reg_var( STRING_varargs_NUM, varargs );
    } else {
        // unnamed args
        for( int i = 0; i < nu; ++i )
            res->ns.reg_var( ast_item->arguments[ i ], vu[ i ] );
        // basic check
        for( int n = 0; n < nn; ++n )
            if ( not ast_item->arguments.contains( names[ n ] ) )
                return res->wr( "named argument that does not appear in def args" );
        // named args
        Vec<bool> used_arg( Size(), nn, false );
        for( int i = nu; i < ast_item->arguments.size(); ++i ) {
            String arg_name = ast_item->arguments[ i ];
            for( int n = 0; ; ++n ) {
                if ( n == nn ) {
                    // not specified arg
                    int j = i - ( ast_item->arguments.size() - ast_item->default_values.size() );
                    if ( j < 0 )
                        return res->wr( "unspecified mandatory argument" );
                    Expr v = ast_item->default_values[ j ]->parse_in( res->ns );
                    res->ns.reg_var( arg_name, v );
                    break;
                }
                if ( arg_name == names[ n ] ) {
                    res->ns.reg_var( arg_name, vn[ n ] );
                    used_arg[ n ] = true;
                    break;
                }
            }
        }
        for( int n = 0; n < nn; ++n )
            if ( not used_arg[ n ] )
                res->ns.disp_error( "arg assigned twice", true );
    }

    // arg constraints
    for( int i = 0; i < ast_item->arg_constraints.size(); ++i ) {
        PRINT( ast_item->arguments[ i ] );
        if ( not ast_item->arg_constraints[ i ].size() )
            continue;
        Expr v = res->ns.get_var( ast_item->arguments[ i ] );
        PRINT( v );
        String n = v->ptype()->orig->ast_item->name;
        if ( int t = ast_item->arg_constraints[ i ].size() ) {
            for( int j = 0; ; ++j ) {
                if ( j == t )
                    return res->wr( "type constraint not fullfilled" );
                if ( ast_item->arg_constraints[ i ][ j ] == n )
                    break;
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
