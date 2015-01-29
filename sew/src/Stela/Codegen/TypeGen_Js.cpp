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


#include "../Ssa/IpSnapshot.h"
#include "../Ssa/Symbol.h"
#include "../Ssa/Room.h"
#include "../Ssa/Type.h"
#include "../Ssa/Op.h"
#include "TypeGen_Js.h"
#include "Codegen_Js.h"
#include "OutReg.h"

TypeGen_Js::TypeGen_Js( Type *type, Stream *ms ) : TypeGen( type ), on( ms ), os( ms ) {
}

void TypeGen_Js::exec() {
    on << "// dep AsmMod.coffee";
    on << "";
    on << Type::AsVar{ type } << " = (function() {";
    on << "    var am = asm_mod.push( function( stdlib, foreign, buffer ) {";
    on << "        \"use asm\";";
    on << "        ";
    on << "        var v_SI8  = new stdlib.Uint8Array  ( buffer );";
    on << "        var v_SI16 = new stdlib.Uint16Array ( buffer );";
    on << "        var v_SI32 = new stdlib.Uint32Array ( buffer );";
    // on << "        var v_SI64 = new stdlib.Uint64Array ( buffer );";
    on << "        var v_PI8  = new stdlib.Int8Array   ( buffer );";
    on << "        var v_PI16 = new stdlib.Int16Array  ( buffer );";
    on << "        var v_PI32 = new stdlib.Int32Array  ( buffer );";
    // on << "        var v_PI64 = new stdlib.Int64Array  ( buffer );";
    on << "        var v_FP32 = new stdlib.Float32Array( buffer );";
    on << "        var v_FP64 = new stdlib.Float64Array( buffer );";
    on << "        ";

    std::map<String,Vec<FuncToGen *> > fmap;

    // asm funcs
    on.nsp += 8;
    for( FuncToGen &f : funcs_to_gen ) {
        fmap[ f.name ] << &f;
        gen_func( f );
    }
    on.nsp -= 8;

    // exports
    on << "        return {";
    for( FuncToGen &f : funcs_to_gen )
        on << "            " << f << ": " << f << ",";
    on << "        };";
    on << "    } );";

    // init
    on << "    function " << Type::AsVar{ type } << "() {";
    on << "        // init;";
    on << "    }";

    //
    for( std::pair<String,Vec<FuncToGen *> > p : fmap ) {
        int max_arg = 0;
        for( FuncToGen *f : p.second )
            max_arg = std::max( max_arg, int( f->arg_types.size() ) );

        on << "    function " << p.first << "() {";
        on << "        // init;";
        on << "    }";
    }

    //    on << "    Object.defineProperty( " << n << ".prototype, \"size_in_bits\", {";
    //    on << "        get: function() {";
    //    on << "            return am.size_in_bits( this.offset );";
    //    on << "        }";
    //    on << "    });";

    on << "    return " << Type::AsVar{ type } << ";";
    on << "})();";
    on << "";

    on << "module.exports = {";
    on << "  " << Type::AsVar{ type } << ": " << Type::AsVar{ type };
    on << "};";
}

void TypeGen_Js::gen_func( FuncToGen &f ) {
    Codegen_Js cjs;
    //    Expr off = symbol( ip->type_SI32, "offset_self" );
    //    Expr pof = room( off );

    // args
    Vec<Expr> args;
    for( Type *t : f.arg_types ) {
        Type *pt = ip->pc->ptr_type_for( t );
        Expr p = symbol( pt, "" );
        p->out_reg = cjs.new_reg( t, "offset_A" + to_string( args.size() ) );
        args << p;
    }


    // func
    Expr var = room( symbol( type, "self" ) );
    IpSnapshot nsv( ip->ip_snapshot );

    Expr res;
    if ( f.name == "init" ) {
        res = ip->pc->apply( ip->pc->type_expr( type ), args.size(), args.ptr() );
    } else {
        Expr func = ip->pc->get_attr( var, f.name );
        if ( func.error() )
            return ip->pc->disp_error( "Impossible to find function named..." );

        res = ip->pc->apply( func, args.size(), args.ptr() );
    }

    PRINT( res );
    // if ( f.name == "init" )

    // signature
    on.write_beg() << "function " << f << "( offset_self";
    for( int i = 0; i < f.arg_types.size(); ++i )
        *on << ", offset_A" << i;
    on.write_end( " ) {" );
    on.nsp += 4;

    on << "offset_self = offset_self | 0;";
    for( int i = 0; i < f.arg_types.size(); ++i )
        on << "offset_A" << i << " = offset_A" << i << " | 0;";

    cjs << res->get();
    cjs.on.nsp = on.nsp;
    cjs.write_to( *on );

    on.nsp -= 4;
    on << "}";

    nsv.undo_parsing_contexts();
    ip->ip_snapshot = nsv.prev;
}
