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

    virtual void clone( Vec<Ptr<Inst> > &created ) const; ///< output in op_mp
    virtual Ptr<Inst> forced_clone( Vec<Ptr<Inst> > &created ) const = 0;

    // graphviz
    static int display_graph( const Vec<ConstPtr<Inst> > &outputs, const char *filename = ".res" );
    virtual void write_graph_rec( Vec<const Inst *> &ext_buf, Stream &os ) const;
    virtual void write_sub_graph_rec( Stream &os ) const;

    //
    virtual Ptr<Inst> _simplified();
    virtual Ptr<Inst> _pointer_on( int beg, int len );

    // parameters
    Vec<Ptr<Inst> > inp;
    Vec<Ptr<Inst> > ext;
    Vec<Ptr<Inst> > exi; ///< for WhileInp
    Vec<Parent>     par;
    Inst           *ext_par;

    //
    static  PI64  cur_op_id; ///<
    mutable PI64  op_id_vis; ///<
    mutable PI64  op_id;     ///< operation id (every new operation on the graph begins with ++current_MO_op_id and one can compare op_id with cur_op_id to see if operation on this node has been done or not).
    mutable void *op_mp;     ///< result of current operations
};

/// uses ip for simplifications
Ptr<Inst> simplified( Ptr<Inst> val );

#endif // INST_H
