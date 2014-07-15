#include "../Codegen/CC_SeqItemContinueOrBreak.h"
#include "../Codegen/CC_SeqItemIf.h"
#include "../Codegen/Codegen_C.h"
#include "While.h"
#include "Ip.h"

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
    virtual void write( Codegen_C *cc, CC_SeqItemBlock **b ) {}
    virtual bool will_write_code() const { return false; }
    Vec<Type *> types;
};

/**
*/
struct WhileOut : Inst {
    WhileOut( const Vec<Vec<bool> > &pos ) : pos( pos ) {}
    virtual void write_dot( Stream &os ) const { os << "while_out"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new WhileOut( pos ); }
    virtual Type *type( int nout ) { return inp[ nout ]->type(); }
    virtual Type *type() { return ip->type_Void; }
    virtual bool need_a_register() { return false; }
    virtual void write( Codegen_C *cc, CC_SeqItemBlock **b ) {}
    virtual bool will_write_code() const { return false; }
    Vec<Vec<bool> > pos;
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

    virtual void get_constraints() {
        PRINT( corr );
        for( int nout = 0; nout < corr.size(); ++nout ) {
            int ninp = corr[ nout ];

            if ( ninp >= 0 ) {
                if ( Inst *p = ext[ 1 ]->find_par_for_nout( ninp ) ) {
                    // inp[ ninp ] of while <-> out[ ninp ] of while_inp
                    this->add_same_out( ninp, p, -1, COMPULSORY );

                    // out[ ninp ] of while_inp <-> inp[ nout ] of while out
                    p->add_same_out( -1, ext[ 0 ].inst, nout, COMPULSORY );
                }
            }

            // inp[ nout ] of while out <-> out[ nout ] of while
            if ( Inst *p = this->find_par_for_nout( nout ) )
                ext[ 0 ]->add_same_out( nout, p, -1, COMPULSORY );
        }
    }

    virtual void add_break_and_continue_internal( CC_SeqItemBlock **b ) {
        Vec<CC_SeqItemBlock *> seq;
        BoolOpSeq cont = get_cont(), need_a_break = not cont, cond = True();
        b[ 0 ]->get_glo_cond_and_seq_of_sub_blocks( seq, cond );

        for( int i = 0; i < seq.size(); ++i ) {
            if ( not seq[ i ]->parent )
                continue; // the main block
            int nb_evicted_blocks = 0;
            if ( not seq[ i ]->parent->ch_followed_by_something_to_execute( nb_evicted_blocks, seq[ i ], seq[ i ]->glo_cond ) ) {
                if ( seq[ i ]->glo_cond.imply( need_a_break ) ) {
                    seq[ i ]->seq << new CC_SeqItemContinueOrBreak( false, seq[ i ] );
                    need_a_break = need_a_break - seq[ i ]->glo_cond; // no need to test seq[ i ]->glo_cond (already done)
                } else if ( nb_evicted_blocks ) {
                    if ( not seq[ i ]->glo_cond.imply( not need_a_break ) ) {
                        // -> if ( rem_to_test ) break;
                        CC_SeqItemIf *ci = new CC_SeqItemIf( seq[ i ] );
                        ci->cond = need_a_break;
                        ci->seq[ 0 ].seq << new CC_SeqItemContinueOrBreak( false, &ci->seq[ 0 ] );
                        seq[ i ]->seq << ci;
                    }
                    // -> continue;
                    seq[ i ]->seq << new CC_SeqItemContinueOrBreak( true, seq[ i ] );
                }
            }
        }

        if ( not need_a_break.always( false ) ) {
            CC_SeqItemIf *cif = new CC_SeqItemIf( b[ 0 ] );
            cif->cond = need_a_break;
            cif->seq[ 0 ].seq << new CC_SeqItemContinueOrBreak( false, &cif->seq[ 0 ] );

            b[ 0 ]->seq << cif;
        }
    }

    virtual void write( Codegen_C *cc, CC_SeqItemBlock **b ) {
        cc->on << "while ( true ) {";
        cc->on.nsp += 4;
        b[ 0 ]->write( cc );
        cc->on.nsp -= 4;
        cc->on << "}";
    }
    BoolOpSeq get_cont() {
        BoolOpSeq cont;
        WhileOut *wout = static_cast<WhileOut *>( ext[ 0 ].inst );
        for( int i = 0, o = corr.size(); i < wout->pos.size(); ++i ) {
            Vec<BoolOpSeq::Item> *s = cont.or_seq.push_back();
            for( int j = 0; j < wout->pos[ i ].size(); ++j, ++o )
                *s << BoolOpSeq::Item{ wout->inp[ o ], wout->pos[ i ][ j ] };
        }
        return cont;
    }

    Vec<int> corr; // output number -> input number or -1
};

Expr while_inp( const Vec<Type *> types ) {
    WhileInp *res = new WhileInp( types );
    return res;
}

Expr while_out( const Vec<Expr> &inp, const Vec<Vec<bool> > &pos ) {
    WhileOut *res = new WhileOut( pos );
    for( Expr i : inp )
        res->add_inp( i );
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
