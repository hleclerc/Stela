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
        for( int ok = 0; ok < 2; ++ok ) {
            // IfOut->inp[ nout ] <-> out of GetNout( If, nout )
            for( int nout = 0; nout < ext[ ok ]->inp.size(); ++nout )
                if ( Inst *p = find_par_for_nout( nout ) )
                    ext[ ok ]->add_same_out( nout, p, -1, COMPULSORY );

            // If->inp[ ninp ] <-> out of GetNout( IfInp, ninp )
            for( int ninp = 0; ninp < inp.size(); ++ninp )
                if ( Inst *p = ext[ 2 + ok ]->find_par_for_nout( ninp ) )
                    add_same_out( ninp, p, -1, COMPULSORY );
        }
    }

    virtual void write( Codegen_C *cc ) {
        cc->on.write_beg() << "if ( ";
        cc->write_out( this->inp[ 0 ] );
        cc->on.write_end( " ) {" );

        cc->on.nsp += 4;
        for( Inst *inst = ext_sched.size() ? ext_sched[ 0 ] : 0; inst; inst = inst->next_sched )
            cc->write( inst );
        cc->on.nsp -= 4;

        cc->on << "} else {";

        cc->on.nsp += 4;
        for( Inst *inst = ext_sched.size() ? ext_sched[ 1 ] : 0; inst; inst = inst->next_sched )
            cc->write( inst );
        cc->on.nsp -= 4;

        cc->on << "}";
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
