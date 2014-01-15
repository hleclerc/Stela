#ifndef INST__H
#define INST__H

#include "Expr.h"

template<int nout,int ninp,int next=0>
class Inst_ : public Inst {
public:
    Inst_() {
        out._size = std::max( 0, nout );
        inp._size = std::max( 0, ninp );
        ext._size = std::max( 0, next );
        out._data = out_data;
        inp._data = inp_data;
        ext._data = ext_data;
    }

    // attributes
    Out out_data[ nout >= 0 ? nout : 0 ];
    Inp inp_data[ ninp >= 0 ? ninp : 0 ];
    Ext ext_data[ next >= 0 ? next : 0 ];

    //
    virtual void _resize_inp( int ns ) {
        if ( ninp >= 0 )
            ASSERT( ninp == ns, "..." );
        else if ( inp._size != ns ) {
            Inst::Inp *nd = new Inst::Inp[ ns ];
            for( int i = 0; i < std::min( ns, inp._size ); ++i )
                nd[ i ] = inp._data[ i ];
            if ( inp._size )
                delete [] inp._data;
            inp._data = nd;
            inp._size = ns;
        }
    }

};

#endif // INST__H
