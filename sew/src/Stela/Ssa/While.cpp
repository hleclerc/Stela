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


#include "GlobalVariables.h"
#include "While.h"

/**
*/
struct WhileInp : Inst {
    WhileInp( const Vec<Type *> types ) : types( types ) {}
    virtual void write_dot( Stream &os ) const { os << "while_inp"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new WhileInp( types ); }
    virtual Type *type( int nout ) { return types[ nout ]; }
    virtual Type *ptype( int nout ) { TODO; return 0; }
    virtual Type *type() { return ip->type_Void; }
    virtual bool need_a_register() { return false; }
    // virtual void write( Codegen_C *cc ) {}
    virtual bool will_write_code() const { return false; }
    Vec<Type *> types;
};

/**
  Last inp is the `cont` (continue) variable
*/
struct WhileOut : Inst {
    virtual void write_dot( Stream &os ) const { os << "while_out"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new WhileOut; }
    virtual Type *type( int nout ) { return inp[ nout ]->type(); }
    virtual Type *type() { return ip->type_Void; }
    virtual bool need_a_register() { return false; }
    // virtual void write( Codegen_C *cc ) {}
    virtual bool will_write_code() const { return false; }
};

/**
*/
struct WhileInst : Inst {
    WhileInst( const Vec<int> &corr ) : corr( corr ) {}
    virtual void write_dot( Stream &os ) const { os << "while"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new WhileInst( corr ); }
    virtual Type *type( int nout ) { return ext[ 0 ]->type( nout ); }
    virtual Type *ptype( int nout ) { TODO; return 0; }
    virtual Type *type() { return ip->type_Void; }
    virtual int ext_disp_size() const { return 1; }
    virtual bool need_a_register() { return false; }

    //    virtual void get_constraints() {
    //        for( int nout = 0; nout < corr.size(); ++nout ) {
    //            int ninp = corr[ nout ];

    //            if ( ninp >= 0 ) {
    //                if ( Inst *p = ext[ 1 ]->find_par_for_nout( ninp ) ) {
    //                    // inp[ ninp ] of while <-> out[ ninp ] of while_inp
    //                    this->add_same_out( ninp, p, -1, COMPULSORY );

    //                    // out[ ninp ] of while_inp <-> inp[ nout ] of while out
    //                    p->add_same_out( -1, ext[ 0 ].inst, nout, COMPULSORY );
    //                }
    //            }

    //            // inp[ nout ] of while out <-> out[ nout ] of while
    //            if ( Inst *p = this->find_par_for_nout( nout ) )
    //                ext[ 0 ]->add_same_out( nout, p, -1, COMPULSORY );
    //        }
    //    }

    //    virtual void add_break_and_continue_internal( CC_SeqItemBlock **b ) {
    //        Vec<CC_SeqItemBlock *> seq;
    //        BoolOpSeq cont = get_cont(), need_a_break = not cont, cond = True();
    //        b[ 0 ]->get_glo_cond_and_seq_of_sub_blocks( seq, cond );

    //        for( int i = 0; i < seq.size(); ++i ) {
    //            if ( not seq[ i ]->parent )
    //                continue; // the main block
    //            int nb_evicted_blocks = 0;
    //            if ( not seq[ i ]->parent->ch_followed_by_something_to_execute( nb_evicted_blocks, seq[ i ], seq[ i ]->glo_cond ) ) {
    //                if ( seq[ i ]->glo_cond.imply( need_a_break ) ) {
    //                    seq[ i ]->seq << new CC_SeqItemContinueOrBreak( false, seq[ i ] );
    //                    need_a_break = need_a_break - seq[ i ]->glo_cond; // no need to test seq[ i ]->glo_cond (already done)
    //                } else if ( nb_evicted_blocks ) {
    //                    if ( not seq[ i ]->glo_cond.imply( not need_a_break ) ) {
    //                        // -> if ( rem_to_test ) break;
    //                        CC_SeqItemIf *ci = new CC_SeqItemIf( seq[ i ] );
    //                        ci->cond = need_a_break;
    //                        ci->seq[ 0 ].seq << new CC_SeqItemContinueOrBreak( false, &ci->seq[ 0 ] );
    //                        seq[ i ]->seq << ci;
    //                    }
    //                    // -> continue;
    //                    seq[ i ]->seq << new CC_SeqItemContinueOrBreak( true, seq[ i ] );
    //                }
    //            }
    //        }

    //        if ( not need_a_break.always( false ) ) {
    //            CC_SeqItemIf *cif = new CC_SeqItemIf( b[ 0 ] );
    //            cif->cond = need_a_break;
    //            cif->seq[ 0 ].seq << new CC_SeqItemContinueOrBreak( false, &cif->seq[ 0 ] );

    //            b[ 0 ]->seq << cif;
    //        }
    //    }

    //    virtual void write( Codegen_C *cc ) {
    //        cc->on << "while ( true ) {";
    //        cc->on.nsp += 4;
    //        TODO;
    //        // b[ 0 ]->write( cc );
    //        cc->on.nsp -= 4;
    //        cc->on << "}";
    //    }

    Expr get_cont() {
        WhileOut *wout = static_cast<WhileOut *>( ext[ 0 ].inst );
        return wout->inp.back();
    }

    Vec<int> corr; // output number -> input number or -1
};

Expr while_inp( const Vec<Type *> &types ) {
    WhileInp *res = new WhileInp( types );
    return res;
}

Expr while_out( const Vec<Expr> &inp, Expr cont ) {
    WhileOut *res = new WhileOut;
    for( Expr i : inp )
        res->add_inp( i );
    res->add_inp( cont );
    return res;
}

Expr while_inst( const Vec<Expr> &inp, Expr winp, Expr wout, const Vec<int> &corr ) {
    WhileInst *res = new WhileInst( corr );
    for( Expr i : inp )
        res->add_inp( i );
    res->add_ext( wout );
    res->add_ext( winp );
    return res;
}

