#include "If.h"

#include "../Codegen/Codegen_C.h"
#include "While.h"
#include "Ip.h"

/**
*/
struct IfInp : Inst {
    IfInp( const Vec<Type *> types ) : types( types ) {}
    virtual void write_dot( Stream &os ) const { os << "if_inp"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new IfInp( types ); }
    virtual Type *type( int nout ) { return types[ nout ]; }
    virtual Type *ptype( int nout ) { TODO; return 0; }
    virtual Type *type() { return ip->type_Void; }
    virtual bool need_a_register() { return false; }
    virtual void write( Codegen_C *cc ) {}
    virtual bool will_write_code() const { return false; }
    Vec<Type *> types;
};

/**
*/
struct IfOut : Inst {
    IfOut() {}
    virtual void write_dot( Stream &os ) const { os << "if_out"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new IfOut; }
    virtual Type *type( int nout ) { return inp[ nout ]->type(); }
    virtual Type *type() { return ip->type_Void; }
    virtual bool need_a_register() { return false; }
    virtual void write( Codegen_C *cc ) {}
    virtual bool will_write_code() const { return false; }
};

/**
*/
struct IfInst : Inst {
    virtual void write_dot( Stream &os ) const { os << "if"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new IfInst; }
    virtual Type *type( int nout ) { return ext[ 0 ]->type( nout ); }
    virtual Type *ptype( int nout ) { TODO; return 0; }
    virtual Type *type() { return ip->type_Void; }
    virtual int ext_disp_size() const { return 2; }
    virtual bool need_a_register() { return false; }

    virtual void get_constraints() {
        TODO;
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
    }

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

    virtual void write( Codegen_C *cc ) {
        TODO;
    }
};

Expr if_inp( const Vec<Type *> &types ) {
    IfInp *res = new IfInp( types );
    return res;
}

Expr if_out( const Vec<Expr> &inp ) {
    IfOut *res = new IfOut;
    for( Expr i : inp )
        res->add_inp( i );
    return res;
}

Expr if_inst( const Vec<Expr> &inp, Expr if_inp_ok, Expr if_inp_ko, Expr if_out_ok, Expr if_out_ko ) {
    IfInst *res = new IfInst;

    for( Expr i : inp )
        res->add_inp( i );

    res->add_ext( if_out_ok );
    res->add_ext( if_out_ko );
    res->add_ext( if_inp_ok );
    res->add_ext( if_inp_ko );

    return res;
}
