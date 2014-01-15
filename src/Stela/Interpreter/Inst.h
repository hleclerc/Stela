#ifndef INST_H
#define INST_H

#include "../System/Stream.h"
#include "../System/Ptr.h"
#include "../System/Vec.h"

/**
*/
class Inst : public ObjectWithCptUse {
public:
    Inst();
    virtual ~Inst();

    virtual const PI8 *cst_data( int nout ) const;
    virtual void write_to_stream( Stream &os ) const = 0;

    struct OutList {
        int size() const { return _size; }
        Out *_data;
        int  _size;
    };

    struct InpList {
        int size() const { return _size; }
        Inp *_data;
        int  _size;
    };

    OutList out;
    InpList inp;
};

#endif // INST_H
