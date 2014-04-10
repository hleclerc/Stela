#ifndef INST_H
#define INST_H

#include "../System/Stream.h"
#include "../System/Ptr.h"
#include "../System/Vec.h"
#include <cstddef>

class InstVisitor;
class BaseType;
class Expr;

/**
*/
class Inst : public ObjectWithCptUse {
public:
    // types
    typedef Expr      Inp;
    typedef ConstPtr<Inst> Ext;

    struct Out {
        struct Parent {
            bool operator==( const Parent &item ) const { return inst == item.inst and ninp == item.ninp; }
            Inst *inst;
            int   ninp;
        };
        Vec<Parent,-1,1> parents;
    };

    enum { ///< very bad...
        #define DECL_INST( INST ) Id_##INST,
        #include "DeclInst.h"
        #undef DECL_INST
        Id__fake_end
    };

    Inst();
    virtual ~Inst();

    virtual int size_in_bits( int nout ) const = 0;
    virtual int size_in_bytes( int nout ) const;
    virtual void write_dot( Stream &os ) const = 0;
    virtual void write_to_stream( Stream &os ) const;
    void mark_children() const;

    virtual const PI8 *cst_data( int nout, int beg, int end ) const;
    virtual const PI8 *vat_data( int nout, int beg, int end ) const;

    // inp
    virtual int inp_size() const = 0;
    virtual void inp_push( Expr var ) = 0;
    virtual void inp_resize( int ns ) = 0;
    virtual void inp_repl( int num, Expr var ) = 0;
    virtual void inp_repl( Expr src, Expr dst ) = 0;
    virtual const Expr &inp_expr( int num ) const = 0;
    virtual Expr &inp_expr( int num ) = 0;

    // out
    virtual int out_size() const = 0;
    virtual Out &out_expr( int n ) = 0;
    virtual const Out &out_expr( int n ) const = 0;

    virtual const BaseType *out_bt( int n ) const;

    // ext
    virtual int ext_size() const = 0;
    virtual void ext_repl( int num, const Inst *inst ) = 0;
    virtual const Inst *ext_inst( int num_ext ) const = 0;
    virtual int ext_size_disp() const;

    //
    virtual void apply( InstVisitor &visitor ) const = 0;
    virtual bool equal( const Inst *b ) const;
    virtual int  inst_id() const = 0; ///< unique id for each inst. very bad...
    virtual int  sizeof_additionnal_data() const; ///< to make clones. very bad
    virtual void copy_additionnal_data_to( PI8 *dst ) const; ///< to make clones. very bad


    static int display_graph( const Vec<ConstPtr<Inst> > &outputs, const char *filename = ".res" );
    virtual void write_graph_rec( Vec<const Inst *> &ext_buf, Stream &os ) const;
    virtual void write_sub_graph_rec( Stream &os ) const;

    /// return inst unless there's already the same operation somewhere
    /// if it's the case, delete inst and return the corresponding instruction
    static Inst *factorized( Inst *inst );

    // methods to construct expressions
    virtual Expr _smp_slice( int nout, int beg, int end );
    virtual Expr _smp_val_at( int nout, int beg, int end );
    virtual Expr _smp_pointer_on( int nout );

    // attributes
    mutable const Inst *ext_parent;
    static  PI64  cur_op_id; ///<
    mutable PI64  op_id_vis; ///<
    mutable PI64  op_id;     ///< operation id (every new operation on the graph begins with ++current_MO_op_id and one can compare op_id with cur_op_id to see if operation on this node has been done or not).
    mutable void *op_mp;     ///< result of current operations
};

#endif // INST_H
