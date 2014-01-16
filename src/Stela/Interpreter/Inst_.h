#ifndef INST__H
#define INST__H

#include "Expr.h"

template<int nb_out,int nb_inp,int nb_ext=0>
class Inst_ : public Inst {
public:
    Inst_() {
    }

    virtual ~Inst_() {
        for( int num = 0; num < inp.size(); ++num )
            inp[ num ].parents().remove_first_unordered( Out::Item{ this, num } );
        for( int num = 0; num < ext.size(); ++num )
            ext[ num ]->ext_parent = 0;
    }

    // inp
    virtual int inp_size() const {
        return inp.size();
    }

    virtual void inp_resize( int ns ) {
        inp.resize( ns );
    }

    virtual void inp_push( Expr var ) {
        int s = inp.size();
        inp.resize( s + 1 );
        inp_repl( s, var );
    }

    virtual void inp_repl( int num, Expr var ) {
        if ( inp[ num ] )
            inp[ num ].parents().remove_first_unordered( Out::Item{ this, num } );
        var.inst->out_expr( var.nout ).parents << Out::Item({ this, num });
        inp[ num ] = var;
    }

    virtual void inp_repl( Expr src, Expr dst ) {
        for( int i = 0; i < inp.size(); ++i )
            if ( inp[ i ] == src )
                inp_repl( i, dst );
    }

    virtual const Expr &inp_expr( int num_inp ) const {
        return inp[ num_inp ];
    }


    // out
    virtual int out_size() const {
        return out.size();
    }

    virtual const Out &out_expr( int n ) const {
        return out[ n ];
    }

    virtual Out &out_expr( int n ) {
        return out[ n ];
    }

    // ext
    virtual int ext_size() const {
        return ext.size();
    }

    virtual void ext_repl( int num, Inst *inst ) {
        ASSERT( inst->ext_parent == 0, "..." );
        inst->ext_parent = this;
        ext[ num ] = inst;
    }

    virtual const Inst *ext_inst( int num_ext ) const {
        return ext[ num_ext ].ptr();
    }

    virtual Inst *ext_inst( int num_ext ) {
        return ext[ num_ext ].ptr();
    }

    Vec<Out,nb_out> out;
    Vec<Inp,nb_inp> inp;
    Vec<Ext,nb_ext> ext;
};

#endif // INST__H
