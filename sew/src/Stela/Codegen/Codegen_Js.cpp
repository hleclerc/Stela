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


#include "../Ssa/GlobalVariables.h"
#include "../Ssa/Symbol.h"
#include "../Ssa/Type.h"
#include "Codegen_Js.h"
#include "InstBlock.h"
#include "OutReg.h"

struct JsRepr {
    void write_to_stream( Stream &os ) const {
        os << *type;
    }
    Type *type;
};

Codegen_Js::Codegen_Js() {
}

static Expr cloned( const Expr &val, Vec<Expr> &created ) {
    val->clone( created );
    return reinterpret_cast<Inst *>( val->op_mp );
}

void Codegen_Js::write_to( Stream &os ) {
    make_code();
    os << ms.str();
}

Vec<Expr> Codegen_Js::make_code() {
    // a clone of the whole hierarchy (enable modifications, parents only from this hierarchy, ...)
    ++Inst::cur_op_id;
    Vec<Expr> out, created;
    for( Expr inst : fresh )
        out << cloned( inst, created );

    // scheduling (and creation of IfInst)
    InstBlock inst_block;
    scheduling( inst_block, out );

    // set missing out_regs
    for( Inst *i = inst_block.beg; i; i = i->next_sched ) {
        if ( not i->out_reg )
            i->out_reg = new_reg( i->type() );
    }

    //
    for( Inst *i = inst_block.beg; i; i = i->next_sched ) {
        i->write( this );
        if ( i->next_sched == 0 and i->out_reg->type != ip->type_Void )
            on << "return " << *i->out_reg << ";";
    }

    return out;
}

void Codegen_Js::exec() {
    TODO;
}

void Codegen_Js::write_beg_cast_bop( Type *type ) {
    if ( ip->is_integer( type ) ) {
        *on << "( ";
    } else {
        TODO;
    }
}

void Codegen_Js::write_end_cast_bop( Type *type ) {
    if ( ip->is_integer( type ) ) {
        *on << " ) | 0";
    } else {
        TODO;
    }
}

void Codegen_Js::write_expr( Expr expr ) {
    on << "    \"use asm\";";

    Codegen_Js cjs;
    cjs.set_os( os, on.nsp + 4 );
    cjs << expr;
    Vec<Expr> res = cjs.make_code();
    cjs.on << "return " << *res[ 0 ]->out_reg << ";";
}

// check if all the children have already been scheduled
static bool ready_to_be_scheduled( Expr inst ) {
    // already in the front ?
    if ( inst->op_id >= Inst::cur_op_id - 1 )
        return false;

    // not computed ?
    for( const Expr &ch : inst->inp )
        if ( ch->op_id < Inst::cur_op_id and not ch->when.always( false ) )
            return false;
    for( const Expr &ch : inst->dep )
        if ( ch->op_id < Inst::cur_op_id and not ch->when.always( false ) )
            return false;

    // ok
    return true;
}


static void get_front( Vec<Expr> &front, Expr inst ) {
    if ( inst->op_id == inst->cur_op_id )
        return;
    inst->op_id = inst->cur_op_id;

    int nb_id = 0;
    for( Expr ch : inst->inp )
        if ( ch )
            nb_id += not ch->when.always( false );
    for( Expr ch : inst->dep )
        if ( ch )
            nb_id += not ch->when.always( false );

    if ( nb_id ) {
        for( Expr ch : inst->inp )
            if ( ch and not ch->when.always( false ) )
                get_front( front, ch );
        for( Expr ch : inst->dep )
            if ( ch and not ch->when.always( false ) )
                get_front( front, ch );
    } else
        front << inst;
}

void Codegen_Js::scheduling( InstBlock &block, Vec<Expr> &out ) {
    // get the front
    ++Inst::cur_op_id;
    Vec<Expr> front;
    for( Expr inst : out )
        get_front( front, inst );
    ++Inst::cur_op_id;
    for( Expr inst : front )
        inst->op_id = Inst::cur_op_id;

    // go to the roots
    ++Inst::cur_op_id;
    while ( front.size() ) {
        // try to find an instruction with the same condition set or an inst that is not going to write anything
        Inst *inst = 0;
        for( int i = 0; i < front.size(); ++i ) {
            inst = front[ i ].inst;
            front.remove_unordered( i );
            break;
        }

        // register
        inst->op_id = Inst::cur_op_id; // say that it's done
        block << inst;

        // update the front
        for( Inst::Parent &p : inst->par ) {
            if ( ready_to_be_scheduled( p.inst ) ) {
                p.inst->op_id = Inst::cur_op_id - 1; // -> in the front
                front << p.inst;
            }
        }
    }
}
