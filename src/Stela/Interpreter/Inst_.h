#ifndef INST__H
#define INST__H

#include "Inst.h"

template<int nout,int ninp,int next=0>
struct Inst_ : Inst {
    Inst_() {
        if ( nout < 0 )
            out._size = 0;
        else {
            out._size = nout;
            out._size = out_data;

        }
        inp._size = inp_size;
        ext._size = ext_size;
    }
};

#endif // INST__H
