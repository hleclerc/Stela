#ifndef INST_H
#define INST_H

#include "../System/Vec.h"
#include <string.h>
#include "Expr.h"
class BoolOpSeq;
class Codegen_C;
class OutReg;
class Type;

/**
*/
class Inst {
public:
    enum {
        TPAR_DEP = -1,
        TPAR_CND = -2
    };
    enum {
        CONST = 1
    };

    struct Parent {
        bool operator==( const Parent &p ) { return inst == p.inst and ninp == p.ninp; }
        Inst *inst;
        int   ninp; ///< input number (or TPAR_...)
    };

    struct Visitor {
        virtual void operator()( Expr expr ) = 0;
    };


    Inst();
    virtual ~Inst();

    virtual void write_to_stream( Stream &os, int prec = -1 ) const;
    virtual void write_dot( Stream &os ) const = 0;


    void add_dep( const Expr &val );
    void add_inp( const Expr &val );
    void mod_inp( const Expr &val, int num );
    void clear_children(); ///< remove inp and dep
    void rem_ref_to_this(); ///< remove inp and dep

    void mark_children();
    virtual void clone( Vec<Expr> &created ) const;
    virtual Expr forced_clone( Vec<Expr> &created ) const = 0;

    template<class T>
    bool get_val( T &res ) { return get_val( &res, 8 * sizeof( res ), 0 ); }

    bool get_val( Bool &res ) { return get_val( &res, 1, 0 ); }

    bool get_val( SI32 &val, Type *type );

    virtual bool get_val( void *dst, int size, int offset = 0, int dst_offset = 0 ) const;

    virtual int sb() const; ///< size in bytes
    virtual int size() const = 0; ///< size in bits
    virtual int size_ptd() const;

    void visit( Visitor &v, bool pointed_data = false, bool want_dep = true );
    virtual void _visit_pointed_data( Visitor &v, bool want_dep );

    virtual int always_checked() const;

    virtual bool has_inp_parent() const;

    virtual bool is_a_pointer() const;
    virtual bool is_a_Select() const;
    virtual bool is_a_Room() const;

    virtual void inp_type_proposition( Type *type, int ninp ); ///< proposition comming from inp[ ninp ]
    virtual void out_type_proposition( Type *type );
    virtual void val_type_proposition( Type *type );
    virtual void update_out_type();

    // graphviz
    static int display_graph( const Vec<Expr> &outputs, const char *filename = ".res" );
    virtual void write_graph_rec( Vec<const Inst *> &ext_buf, Stream &os ) const;
    virtual void write_sub_graph_rec( Stream &os ) const;

    virtual void write_to( Codegen_C *cc, int prec = -1 ); ///< prec = -1 -> make a new line for this instruction
    virtual void write_to( Codegen_C *cc, int prec, OutReg *out_reg );
    virtual bool going_to_write_c_code();

    virtual void update_when( const BoolOpSeq &cond );

    virtual void _add_store_dep_if_necessary( Expr res, Expr fut );
    virtual Expr _simplified();
    virtual Expr _get_val();
    virtual Expr _get_val( int len );
    virtual void _set_val( Expr val, int len );

    virtual Expr _simp_slice( int off, int len );

    virtual BoolOpSeq get_BoolOpSeq();

    Vec<Expr>           inp; ///< inputs
    Vec<Expr>           dep; ///< dependencies
    Vec<Expr>           ext; ///< WhileOut, ...
    mutable Vec<Parent> par; ///< parents
    BoolOpSeq          *when; ///< used for code generation (to know when needed)

    Inst               *ext_par;

    int                 flags;

    static  PI64        cur_op_id; ///<
    mutable PI64        op_id_vis; ///<
    mutable PI64        op_id;     ///< operation id (every new operation on the graph begins with ++current_MO_op_id and one can compare op_id with cur_op_id to see if operation on this node has been done or not).
    mutable void       *op_mp;     ///< result of current operations
    mutable int         cpt_use;
};

// inlined Expr methods
#include "_Expr_defs.h"

#endif // INST_H
