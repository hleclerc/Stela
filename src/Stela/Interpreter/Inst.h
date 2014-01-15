#ifndef INST_H
#define INST_H

#include "../System/Stream.h"
#include "../System/Ptr.h"
#include "../System/Vec.h"
struct Expr;

/**
*/
class Inst : public ObjectWithCptUse {
public:
    // types
    typedef Expr  Inp;
    typedef Inst *Ext;

    struct Out {
        struct Item {
            Inst *inst;
            int   ninp;
        };
        Vec<Item,-1,1> parents;
    };

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

    struct ExtList {
        int size() const { return _size; }
        Ext *_data;
        int  _size;
    };

    // methods
    Inst();
    virtual ~Inst();

    virtual const PI8 *cst_data( int nout ) const;
    virtual void write_to_stream( Stream &os ) const = 0;

    // attributes
    OutList out;
    InpList inp;
    ExtList ext;
};

#endif // INST_H
