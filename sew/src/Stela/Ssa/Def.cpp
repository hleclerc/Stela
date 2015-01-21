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
#include "../Ast/Ast_Def.h"
#include "Varargs.h"
#include "Slice.h"
#include "Class.h"
#include "Room.h"
#include "Type.h"
#include "Inst.h"
#include "Def.h"
#include "Op.h"

Def::TrialDef::TrialDef( ParsingContext *caller, Def *orig ) : orig( orig ), ns( 0, caller, "TrialDef_" + to_string( orig ) ) {
    // ns.catched_vars = &orig->catched_vars;
}

Def::TrialDef::~TrialDef() {
}

Expr Def::TrialDef::call( int nu, Expr *vu, int nn, const String *names, Expr *vn, int pnu, Expr *pvu, int pnn, const String *pnames, Expr *pvn, int apply_mode, ParsingContext *caller, const Expr &cond, Expr self, Varargs *va_size_init ) {
    if ( apply_mode != ParsingContext::APPLY_MODE_STD )
        TODO;

    //
    ns.cond = and_boolean( ns.cond, cond );

    // particular case
    if ( orig->ast_item->name == "init" and not self.error() ) {
        Type *self_type = self->ptype();

        if ( self_type->size() < 0 )
            ASSERT( va_size_init, "If size is not known, expecting a va_size_init" );

        Vec<Expr> offsets, lengths;

        // ancestors
        const Ast_Class *ac = static_cast<const Ast_Class *>( self_type->orig->ast_item );
        if ( ac->inheritance.size() )
            TODO;

        // offsets
        Expr off( 0 );
        for( Type::Attr &a : self_type->attributes ) {
            if ( a.off_expr ) {
                int ali = a.val->ptype()->alig();
                if ( a.val->ptype() == ip->type_Repeated ) {
                    SI64 rep_dat[ 2 ]; if ( not a.val->get()->get_val( rep_dat, 2 * 64 ) ) ERROR( "..." );
                    Type *rep_type = reinterpret_cast<Type *>( (ST)rep_dat[ 0 ] );
                    ali = rep_type->alig();
                }

                // alig
                Expr args[] = { room( off ), room( ali ) };
                off = caller->apply( ns.get_var( "ceil" ), 2, args );
                if ( not off )
                    return off;
                off = off->get( caller->cond );

                // reg offset
                offsets << off;

                // add length
                Expr arga[] = { room( off ), Expr() };

                int s = a.val->ptype()->size();
                if ( s < 0 or a.val->ptype() == ip->type_Repeated ) {
                    ASSERT( va_size_init, "..." );
                    for( int i = 0; ; ++i ) {
                        if ( i == va_size_init->names.size() )
                            return caller->ret_error( "attribute of variable size does not have a size_init value" );
                        if ( va_size_init->names[ i ] == a.name ) {
                            arga[ 1 ] = va_size_init->exprs[ i ];
                            break;
                        }
                    }
                } else
                    arga[ 1 ] = room( s );

                lengths << arga[ 1 ];
                off = ns.apply( ns.get_var( "add" ), 2, arga )->get( ns.cond );
            } else {
                offsets << Expr();
                lengths << Expr();
            }
        }
        // for( auto p : offsets )
        //     PRINT( p.second );

        // : attr( val ), ...
        const Ast_Def *ad = static_cast<const Ast_Def *>( orig->ast_item );
        Vec<Bool> initialised_args( Size(), self_type->attributes.size(), false );
        for( const Ast_Def::StartsWith_Item &d : ad->starts_with ) {
            for( int i = 0; ; ++i ) {
                if ( i == self_type->attributes.size() )
                    return ns.ret_error( "no attribute " + d.attr + " in class" );

                Type::Attr &a = self_type->attributes[ i ];
                if ( d.attr == a.name ) {
                    // parse args
                    Vec<Expr> args;
                    int nu = d.args.size() - d.names.size();
                    for( int i = 0; i < d.args.size(); ++i )
                        args << d.args[ i ]->parse_in( ns );

                    //
                    ns.call_init( self_type->attr_expr( self, a ), nu, args.ptr(), d.names.size(), d.names.ptr(), args.ptr() + nu );
                    initialised_args[ i ] = true;
                    break;
                }
            }
        }

        // default initialization
        int cpt = 0;
        for( int num_attr = 0; num_attr < self_type->attributes.size(); ++num_attr ) {
            Type::Attr &a = self_type->attributes[ num_attr ];
            if ( a.off_expr and initialised_args[ cpt ] == false ) {
                ns.current_off = a.off;
                ns.current_src = a.src;

                if ( a.val->ptype() == ip->type_Repeated ) {
                    // call ___repeated_init rep, var, len, varargs
                    Vec<Expr>   init_exprs;
                    Vec<String> init_names;
                    Expr args[] = { a.val, add( self, offsets[ num_attr ] ), lengths[ num_attr ], ns._make_varars( init_exprs, init_names ) };
                    ns.apply( ns.get_var( "___repeated_init" ), 4, args );
                } else
                    ns.call_init( self_type->attr_expr( self, a ), not ( a.val->flags & Inst::PART_INST ), &a.val );
            }
        }
        ++cpt;
    }

    // inline call
    return orig->ast_item->block->parse_in( ns );
}

Def::Def( const Ast_Callable *ast_item ) : Callable( ast_item ) {
}

Callable::Trial *Def::test( int nu, Expr *vu, int nn, const String *names, Expr *vn, int pnu, Expr *pvu, int pnn, const String *pnames, Expr *pvn, ParsingContext *caller, Expr self ) {
    //
    if ( pnu + pnn )
        TODO; // make a vu and names list

    //
    TrialDef *res = new TrialDef( caller, this );
    if ( self ) {
        res->ns.reg_var( "self", self );
        res->ns.self = self;
    }

    //
    if ( ast_item->pertinence )
        TODO;

    // nb arguments
    if ( pnu + pnn + nu + nn < ast_item->min_nb_args() ) return res->wr( "no enough arguments" );
    if ( pnu + pnn + nu + nn > ast_item->max_nb_args() ) return res->wr( "To much arguments" );


    Vec<Expr  > v_args;
    Vec<String> v_names;

    // unnamed args
    int beg_va = std::min( nu, (int)ast_item->arguments.size() );
    for( int i = 0; i < beg_va; ++i )
        res->ns.reg_var( ast_item->arguments[ i ], vu[ i ] );
    for( int i = beg_va; i < nu; ++i )
        v_args << vu[ i ];

    // basic check for named args
    if ( not ast_item->varargs )
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

    for( int n = 0; n < nn; ++n ) {
        if ( not used_arg[ n ] ) {
            v_args  << vn[ n ];
            v_names << names[ n ];
        }
    }

    if ( ast_item->varargs )
        res->ns.reg_var( "varargs", res->ns._make_varars( v_args, v_names ) );
    else if ( v_args.size() )
        return res->wr( "arg name not in the declaration" );

    // arg constraints
    for( int i = 0; i < ast_item->arg_constraints.size(); ++i ) {
        if ( not ast_item->arg_constraints[ i ].size() )
            continue;
        Expr v = res->ns.get_var( ast_item->arguments[ i ] );
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
