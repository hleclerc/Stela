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


#include "../Ssa/Symbol.h"
#include "../Ssa/Type.h"
#include "Codegen_Js.h"
#include "OutReg.h"

struct JsRepr {
    void write_to_stream( Stream &os ) const {
        os << *type;
    }
    Type *type;
};

Codegen_Js::Codegen_Js() {
}

void Codegen_Js::gen_type( Stream &out, Type *type ) {
    String n = to_string( JsRepr{ type } );
    Expr obj = symbol( type, "this" );
    set_os( &out );

    on << n << " = (function() {";
    on << "    var am = asm_mod.push( function( stdlib, foreign, buffer ) {";
    on << "        \"use asm\";";
    on << "        return {";
    on << "            size_in_bits: size_in_bits,";
    on << "            get_val     : get_val,";
    on << "            set_val     : set_val,";
    on << "            init        : init";
    on << "        };";
    on << "    } );";

    on << "    function " << n << "( offset, parent ) {";
    on << "        this.offset = offset == null ? am.allocate( 1 ) : offset;";
    on << "        this.parent = parent == null ? undefined : parent;";
    on << "        am.init( this.offset );";
    on << "    }";

    on << "    Object.defineProperty( " << n << ".prototype, \"size_in_bits\", {";
    on << "        get: function() {";
    on << "            return am.size_in_bits( this.offset );";
    on << "        }";
    on << "    });";

    on << "    return " << n << ";";
    on << "})();";




    on << "function " << n << "( array_buffer, offset ) {";
    on << "    this.array_buffer = array_buffer == null ? new ArrayBuffer( 1 ) : array_buffer;";
    on << "    this.offset = offset == null ? 0 : offset;";
    on << "}";
    on << n << ".prototype.size_in_bits = function() {";
    write_expr( obj->size() );
    on << "}";

    on << "module.exports = {";
    on << "  " << n << ": " << n;
    on << "};";
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
    Inst *beg = scheduling( out );

    // variables (registers)
    for( Inst *i = beg; i; i = i->next_sched ) {
        i->out_reg = new_reg();
    }

    //
    for( Inst *i = beg; i; i = i->next_sched ) {
        i->write( this );
    }

    return out;
}

void Codegen_Js::exec() {
    TODO;
}

AutoPtr<Codegen::Writable> Codegen_Js::var_decl( OutReg *reg ) {
    struct VarDeclJS : Codegen::Writable {
        VarDeclJS( OutReg *reg ) : reg( reg ) {}
         virtual void write_to_stream( Stream &os ) const {
            os << "var " << *reg;
        }
        OutReg *reg;
    };
    return new VarDeclJS( reg );
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

Inst *Codegen_Js::scheduling( Vec<Expr> &out ) {
    // get the front
    ++Inst::cur_op_id;
    Vec<Expr> front;
    for( Expr inst : out )
        get_front( front, inst );
    ++Inst::cur_op_id;
    for( Expr inst : front )
        inst->op_id = Inst::cur_op_id;

    // go to the roots
    Inst *beg = 0, *end;
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
        if ( not beg ) {
            beg = inst;
            end = inst;
        } else {
            end->next_sched = inst;
            inst->prev_sched = end;
            end = inst;
        }

        // update the front
        for( Inst::Parent &p : inst->par ) {
            if ( ready_to_be_scheduled( p.inst ) ) {
                p.inst->op_id = Inst::cur_op_id - 1; // -> in the front
                front << p.inst;
            }
        }
    }

    return beg;
}
