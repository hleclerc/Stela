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

#include "../Codegen/Codegen.h"
#include "ParsingContext.h"
#include "IpSnapshot.h"
#include "Type.h"
#include "Inst.h"

#include <fstream>

PI64 Inst::cur_op_id = 0;

Inst::Inst() {
    ext_par           = 0;
    op_id_vis         = 0;
    op_id             = 0;
    op_mp             = 0;
    cpt_use           = 0;
    flags             = 0;
    next_sched        = 0;
    prev_sched        = 0;
    out_reg           = 0;
}

Inst::~Inst() {
    rem_ref_to_this();
}

void Inst::write_to_stream( Stream &os, int prec ) {
    if ( Type *t = type() )
        os << '{' << *t << '}';
    write_dot( os );
    if ( inp.size() ) {
        for( int i = 0; i < inp.size(); ++i )
            os << ( i ? "," : "(" ) << inp[ i ];
        os << ")";
    }
}

void Inst::set( Expr obj, Expr cond ) {
    PRINT( *this );
    ip->pc->disp_error( "setting a non pointer item" );
}

Expr Inst::get( Expr cond ) {
    PRINT( *this );
    return ip->pc->ret_error( "getting a non pointer item" );
}

Expr Inst::simplified( Expr cond ) {
    return this;
}

int Inst::pointing_to_nout() {
    return 0;
}

Type *Inst::type( int nout ) {
    PRINT( *this );
    ERROR( "..." );
    return 0;
}

Type *Inst::ptype( int nout ) {
    ip->pc->disp_error( "Not a pointer type" );
    ERROR( "Not a pointer type" );
    return 0;
}

Type *Inst::ptype() {
    ip->pc->disp_error( "Not a pointer type" );
    ERROR( "Not a pointer type" );
    return 0;
}

Type *Inst::type() {
    PRINT( *this );
    TODO;
    return 0;
}

Expr Inst::size() {
    if ( Type *t = type() )
        return t->size( this );
    return Expr();
}

bool Inst::get_val( void *res, Type *type ) {
    return false;
}

bool Inst::get_val( void *res, int size ) {
    return false;
}

void Inst::write( Codegen *c ) {
    c->on << "// TODO: write " << *this;
}

bool Inst::is_surdef() const {
    return flags & SURDEF;
}

bool Inst::is_const() const {
    return flags & CONST;
}

int Inst::op_type() const {
    return -1;
}

bool Inst::always( bool val ) const {
    return false;
}

bool Inst::always_equal( Type *t, const void *d ) {
    return false;
}

Vec<Expr> Inst::subs( Vec<Expr> &expr_list, Vec<Expr> &src, Vec<Expr> &dst ) {
    ++Inst::cur_op_id;
    ASSERT( src.size() == dst.size(), "..." );
    for( int i = 0; i < src.size(); ++i ) {
        src[ i ]->op_id = Inst::cur_op_id;
        src[ i ]->op_mp = dst[ i ].inst;
    }
    Vec<Expr> created, res;
    for( Expr e : expr_list ) {
        e->subs( created );
        res << reinterpret_cast<Inst *>( e->op_mp );
    }
    return res;
}

Expr Inst::subs( Vec<Expr> &src, Vec<Expr> &dst ) {
    Vec<Expr> expr_list( this ), res( subs( expr_list, src, dst ) );
    return res[ 0 ];
}

Expr Inst::subs( Expr src, Expr dst ) {
    Vec<Expr> s( src ), d( dst );
    return subs( s, d );
}

void Inst::subs( Vec<Expr> &created ) {
    if ( op_id == cur_op_id )
        return;
    op_id = cur_op_id;

    bool change_inp = false, change_ext = false, change_dep = false;
    for( Expr e : inp ) {
        e->subs( created );
        change_inp |= e->op_mp != e.inst;
    }
    for( Expr e : ext ) {
        e->subs( created );
        change_ext |= e->op_mp != e.inst;
    }
    for( Expr e : dep ) {
        e->subs( created );
        change_dep |= e->op_mp != e.inst;
    }

    if ( change_inp or change_ext or change_dep ) {
        Expr res = _subs();
        op_mp = res.inst;
        created << res;

        for( Expr e : dep )
            res->add_dep( e->op_mp );
    } else
        op_mp = this;
}

#define DECL_BT( T ) bool Inst::always_equal( T val ) { return always_equal( ip->type_##T, &val ); }
#include "DeclArytTypes.h"
#undef DECL_BT

Expr Inst::_simp_repl_bits( Expr off, Expr val ) {
    return Expr();
}

Expr Inst::_simp_slice( Type *dst, Expr off ) {
    return Expr();
}

Expr Inst::_simp_rcast( Type *dst ) {
    if ( type() == dst )
        return this;
    return Expr();
}

Inst *Inst::twin_or_val( Inst *inst ) {
    //    if ( inst->inp.size() ) {
    //        const Vec<Inst::Out::Parent,-1,1> &p = inst->inp[ 0 ].parents();
    //        for( Parent &p : inp[ 0 ]->parents ) {
    //            if ( p.inst != inst and p.inst->eq_twin_or_val( inst ) ) {
    //                delete inst;
    //                return p.inst;
    //            }
    //        }
    //    }
    return inst;
}

void Inst::add_dep( const Expr &val ) {
    val->par << Parent{ this, TPAR_DEP };
    dep << val;
}

void Inst::add_inp( const Expr &val ) {
    if ( val )
        val->par << Parent{ this, int( inp.size() ) };
    inp << val;
}

void Inst::add_ext( const Expr &val ) {
    val->ext_par = this;
    ext << val;
}

void Inst::mod_inp( const Expr &val, int num ) {
    if ( inp.size() <= num )
        inp.resize( num + 1 );
    inp[ num ]->par.remove_first_unordered( Parent{ this, num } );
    val->par << Parent{ this, num };
    inp[ num ] = val;
}

void Inst::mod_dep( const Expr &val, Inst *d ) {
    for( int i = 0; i < dep.size(); ++i ) {
        if ( dep[ i ] == d ) {
            d->par.remove_first_unordered( Parent{ this, -1 } );
            val->par << Parent{ this, -1 };
            dep[ i ] = val;
            break;
        }
    }
}

void Inst::rem_dep( Inst *d ) {
    for( int i = 0; i < dep.size(); ++i ) {
        if ( dep[ i ] == d ) {
            d->par.remove_first_unordered( Parent{ this, -1 } );
            dep.remove( i );
            break;
        }
    }
}

void Inst::add_store_dep( Inst *dst ) {
    if ( op_id == cur_op_id )
        return;
    op_id = cur_op_id;
    _mk_store_dep( dst );
    for( Expr &e : inp )
        e->add_store_dep( dst );
}

void Inst::rem_ref_to_this() {
    for( int num = 0; num < inp.size(); ++num )
        if ( inp[ num ] )
            inp[ num ]->par.remove_first_unordered( Parent{ this, num } );
    for( int num = 0; num < dep.size(); ++num )
        if ( dep[ num ] )
            dep[ num ]->par.remove_first_unordered( Parent{ this, TPAR_DEP } );
    for( int num = 0; num < ext.size(); ++num )
        if ( ext[ num ] )
            ext[ num ]->ext_par = 0;
}

void Inst::mark_children( Vec<Expr> *seq ) {
    if ( op_id == cur_op_id )
        return;
    op_id = cur_op_id;

    if ( seq )
        *seq << this;

    for( Expr &e : inp )
        if ( e )
            e->mark_children( seq );
    for( Expr &e : dep )
        if ( e )
            e->mark_children( seq );
    for( Expr &e : ext )
        if ( e )
            e->mark_children( seq );
}


void Inst::clone( Vec<Expr> &created ) const {
    if ( op_id == cur_op_id )
        return;
    op_id = cur_op_id;

    // clone the children
    for( const Expr &i : inp )
        i->clone( created );
    for( const Expr &i : dep )
        i->clone( created );
    for( const Expr &i : ext )
        i->clone( created );

    // basic clone
    Expr res = forced_clone( created );

    // add cloned children
    for( const Expr &i : inp )
        res->add_inp( reinterpret_cast<Inst *>( i->op_mp ) );
    for( const Expr &i : dep )
        res->add_dep( reinterpret_cast<Inst *>( i->op_mp ) );
    for( const Expr &i : ext )
        res->add_ext( reinterpret_cast<Inst *>( i->op_mp ) );

    // register
    op_mp = res.inst;
    created << res;
}

int Inst::display_graph( Vec<Expr> outputs, const char *filename ) {
    std::ofstream f( filename );
    f << "digraph Instruction {\n";
    f << "  node [shape = record];\n";

    // get the nodes (within clusters)
    ++cur_op_id;
    Vec<Inst *> ext_buf, seq;
    for( int i = 0; i < outputs.size(); ++i )
        outputs[ i ]->write_graph_rec( ext_buf, seq, f );
    for( Inst *ch : ext_buf )
        if ( ch )
            ch->write_sub_graph_rec( seq, f );

    // edges
    for( Inst *i : seq ) {
        // ext
        for( int e = 0; e < i->ext_disp_size(); ++e )
            f << "    node" << i << " -> node" << i->ext[ e ].inst << " [color=\"green\"];\n";

        // sched
        //if ( Inst *n = i->next_sched )
        //    f << "    node" << n << " -> node" << i << " [color=\"yellow\"];\n";

        // parents
        //    for( int i = 0; i < par.size(); ++i ) {
        //        if ( par[ i ].ninp >= 0 and par[ i ].inst->inp.size() > 1 )
        //            os << "    node" << par[ i ].inst << ":f" << par[ i ].ninp << " -> node" << this << " [color=red,style=dotted];\n";
        //        else
        //            os << "    node" << par[ i ].inst << " -> node" << this << " [color=red,style=dotted];\n";
        //    }
        //if ( par_ext_sched )
        //    os << "    node" << par_ext_sched << " -> node" << this << " [color=yellow,style=dotted];\n";
    }


    f << "}";
    f.close();

    return system( ( "dot -Tps " + std::string( filename ) + " > " + std::string( filename ) + ".eps && gv " + std::string( filename ) + ".eps" ).c_str() );
}

void Inst::write_sub_graph_rec( Vec<Inst *> &seq, Stream &os ) {
    os << "    subgraph cluster_" << this <<" {\ncolor=yellow;\nstyle=dotted;\n";
    Vec<Inst *> ext_buf;
    write_graph_rec( ext_buf, seq, os );
    for( Inst *nch : ext_buf )
        if ( nch )
            nch->write_sub_graph_rec( seq, os );
    os << "    }\n";
}

void Inst::write_graph_rec( Vec<Inst *> &ext_buf, Vec<Inst *> &seq, Stream &os ) {
    if ( op_id_vis == cur_op_id )
        return;
    op_id_vis = cur_op_id;
    seq << this;

    // label
    std::ostringstream ss;
    write_dot( ss );

    //if ( when )
    //    when->write_to_stream( ss << "\n" );
    //if ( out_reg )
    //    ss << ".R" << out_reg->num;
    // ss << " " << sched_num;
    //if ( IIC( this )->out_reg )
    //    IIC( this )->out_reg->write_to_stream( ss << " " );

    // node
    std::string ls = ss.str();
    os << "    node" << this << " [label=\"";
    for( unsigned i = 0; i < ls.size(); ++i ) {
        switch ( ls[ i ] ) {
        case '<':
        case '>':
        case '\\':
            os << '\\';
        }
        os << ls[ i ];
    }
    if ( inp.size() > 1 )
        for( int i = 0; i < inp.size(); ++i )
            os << "|<f" << i << ">i";
    os << "\"];\n";

    // children
    for( int i = 0; i < inp.size(); ++i ) {
        if ( inp[ i ] ) {
            os << "    node" << this;
            if ( inp.size() > 1 )
                os << ":f" << i;
            os << " -> node" << inp[ i ].inst << ";\n";

            inp[ i ]->write_graph_rec( ext_buf, seq, os );
        }
    }

    // dependencies
    for( int i = 0; i < dep.size(); ++i ) {
        if ( dep[ i ] ) {
            os << "    node" << this << " -> node" << dep[ i ].inst << " [style=dotted];\n";
            dep[ i ]->write_graph_rec( ext_buf, seq, os );
        }
    }

    // ext
    for( int i = 0; i < ext_disp_size(); ++i )
        if ( Inst *ch = ext[ i ].inst )
            ext_buf << ch;
}

int Inst::ext_disp_size() const {
    return ext.size();
}

void Inst::_mk_store_dep( Inst *dst ) {
}
