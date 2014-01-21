#ifndef INST_H
#define INST_H

#include "../System/Stream.h"
#include "../System/Ptr.h"
#include "../System/Vec.h"
#include <cstddef>

struct InstVisitor;
struct Expr;

/**
*/
class Inst : public ObjectWithCptUse {
public:
    // types
    typedef Expr      Inp;
    typedef Ptr<Inst> Ext;

    struct Out {
        struct Item {
            bool operator==( const Item &item ) const { return inst == item.inst and ninp == item.ninp; }
            Inst *inst;
            int   ninp;
        };
        Vec<Item,-1,1> parents;
    };

    Inst();
    virtual ~Inst();

    virtual int size_in_bits( int nout ) const = 0;
    virtual int size_in_bytes( int nout ) const;
    virtual const PI8 *cst_data( int nout ) const;
    virtual void write_to_stream( Stream &os ) const = 0;

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

    // ext
    virtual int ext_size() const = 0;
    virtual void ext_repl( int num, Inst *inst ) = 0;
    virtual const Inst *ext_inst( int num_ext ) const = 0;
    virtual Inst *ext_inst( int num_ext ) = 0;

    //
    virtual void apply( InstVisitor &visitor ) const = 0;
    virtual bool equal( const Inst *b ) const;
    virtual int  inst_id() const = 0; ///< unique id for each inst

    /// return inst unless there's already the same operation somewhere
    /// if it's the case, delete inst and return the corresponding instruction
    static Inst *factorized( Inst *inst );

    // methods to construct expressions
    virtual Expr _smp_slice( int nout, int beg, int end );
    virtual Expr _smp_val_at( int nout, int size );
    virtual Expr _smp_pointer_on( int nout );

    // attributes
    Inst         *ext_parent;
    static  PI64  cur_op_id; ///<
    mutable PI64  op_id_viz; ///<
    mutable PI64  op_id;     ///< operation id (every new operation on the graph begins with ++current_MO_op_id and one can compare op_id with cur_op_id to see if operation on this node has been done or not).
    mutable void *op_mp;     ///< result of current operations
};

#endif // INST_H
