#ifndef INST_H
#define INST_H

#include "../System/Stream.h"
#include "../System/Ptr.h"
#include "../System/Vec.h"
class Var;

/**
*/
class Inst : public ObjectWithCptUse {
public:
    struct Parent {
        Inst *inst;
        int   ninp; ///< input number
    };

    virtual ~Inst();
    virtual void write_to_stream( Stream &os ) const;
    virtual void write_dot( Stream &os ) const = 0;
    virtual int size() const = 0;


    virtual void set( Ptr<Inst> val );
    virtual void add_var_ptr( Var *var );

    void add_inp( Ptr<Inst> val );

    virtual Ptr<Inst> _simplified();
    virtual Ptr<Inst> _pointer_on( int beg, int len );

    Vec<Ptr<Inst> > inp;
    Vec<Ptr<Inst> > ext;
    Vec<Parent>     par;
};

Ptr<Inst> simplified( Ptr<Inst> val );

#endif // INST_H
