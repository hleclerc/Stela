#ifndef INST_H
#define INST_H

#include "../System/Vec.h"
#include "Expr.h"
class BoolOpSeq;
class Type;

/**
*/
class Inst {
public:
    struct Parent {
        bool operator==( const Parent &p ) { return inst == p.inst and ninp == p.ninp; }
        Inst *inst;
        int   ninp; ///< input number (or TPAR_...)
    };
    enum {
        CONST = 1,
        SURDEF = 2
    };
    enum {
        TPAR_DEP = -1,
    };

    Inst();
    virtual ~Inst();

    virtual void set( Expr obj, const BoolOpSeq &cond ); ///< set pointed value
    virtual Expr get( const BoolOpSeq &cond ); ///< get pointed value
    virtual Expr get(); ///< get pointed value
    virtual Expr simplified( const BoolOpSeq &cond );
    virtual bool same_cst( const Inst *inst ) const;
    virtual bool emas_cst( const Inst *inst ) const;
    virtual Expr size();

    void add_dep( const Expr &val );
    void add_inp( const Expr &val );
    void add_ext( const Expr &val );

    void mod_inp( const Expr &val, int num );

    void add_store_dep( Inst *dst );

    virtual void write_to_stream( Stream &os, int prec = -1 );
    virtual void write_dot( Stream &os ) = 0;
    virtual Type *type() = 0;
    virtual Type *ptype();

    void rem_ref_to_this();

    void mark_children();

    virtual void clone( Vec<Expr> &created ) const;
    virtual Expr forced_clone( Vec<Expr> &created ) const = 0;

    // properties
    virtual int always_checked() const;
    virtual bool has_inp_parent() const;

    virtual bool uninitialized() const;
    virtual bool is_surdef() const;
    virtual bool is_const() const;

    virtual bool get_val( Type *type, void *data ) const;
    virtual operator BoolOpSeq() const;

    // display
    static int display_graph( Vec<Expr> outputs, const char *filename = ".res" );
    virtual void write_graph_rec( Vec<Inst *> ext_buf, Stream &os );
    virtual void write_sub_graph_rec( Stream &os );
    virtual int ext_disp_size() const;

    virtual Expr _simp_repl_bits( Expr off, Expr val );
    virtual Expr _simp_slice( Type *dst, Expr off );
    virtual void _mk_store_dep( Inst *dst );

    Vec<Expr>           inp;
    Vec<Expr>           ext;
    Vec<Expr>           dep;
    mutable Vec<Parent> par; ///< parents
    mutable Inst       *ext_par;
    BoolOpSeq          *when; ///< used for code generation (to know when needed)

    int                 flags;

    static  PI64        cur_op_id; ///<
    mutable PI64        op_id_vis; ///<
    mutable PI64        op_id;     ///< operation id (every new operation on the graph begins with ++current_MO_op_id and one can compare op_id with cur_op_id to see if operation on this node has been done or not).
    mutable void       *op_mp;     ///< result of current operations
    mutable int         cpt_use;
};

#endif // INST_H
