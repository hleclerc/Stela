#ifndef INST_H
#define INST_H

#include "../System/Vec.h"
#include "Expr.h"
#include <map>
#include <set>
class CppOutReg;
class BoolOpSeq;
class Codegen_C;
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
    ///< flags
    enum {
        CONST     = 1,
        SURDEF    = 2,
        PART_INST = 4 // partial instanciation
    };
    ///< for Parent::ninp
    enum {
        TPAR_DEP = -1
    };
    ///< for same_out and diff_out level
    enum {
        OPTIONNAL  = 0,
        COMPULSORY = 100 ///< for same_out and diff_out
    };
    ///< for same_out and diff_out level
    struct PortConstraint {
        bool operator<( const PortConstraint &p ) const {
            if ( src_ninp != p.src_ninp ) return src_ninp < p.src_ninp;
            if ( dst_inst != p.dst_inst ) return dst_inst < p.dst_inst;
            return dst_ninp < p.dst_ninp;
        }
        int   src_ninp; ///< -1 -> out
        Inst *dst_inst;
        int   dst_ninp; ///< -1 -> out
    };

    Inst();
    virtual ~Inst();

    virtual void set( Expr obj, const BoolOpSeq &cond ); ///< set pointed value
    virtual Expr get( const BoolOpSeq &cond ); ///< get pointed value
    virtual Expr get(); ///< get pointed value
    virtual Expr simplified( const BoolOpSeq &cond );
    virtual bool same_cst( const Inst *inst ) const;
    virtual bool emas_cst( const Inst *inst ) const;
    virtual SI32 size();

    void add_dep( const Expr &val );
    void add_inp( const Expr &val );
    void add_ext( const Expr &val );

    void mod_inp( const Expr &val, int num );

    void add_store_dep( Inst *dst );

    virtual void write_to_stream( Stream &os, int prec = -1 );
    virtual void write_dot( Stream &os ) const = 0;
    virtual Type *type( int nout ); ///
    virtual Type *ptype( int nout );
    virtual Type *type() = 0;
    virtual Type *ptype();

    void rem_ref_to_this();
    void replace_this_by_inp( int ninp, Vec<Expr> &out );
    Inst *find_par_for_nout( int nout );
    virtual int pointing_to_nout();

    void mark_children( Vec<Expr> *seq = 0 );

    virtual void clone( Vec<Expr> &created ) const;
    virtual Expr forced_clone( Vec<Expr> &created ) const = 0;


    // properties
    virtual bool has_inp_parent( Inst *inst ) const;
    virtual bool has_inp_parent() const;
    virtual int  always_checked() const;
    virtual int  nb_inp_parents() const;

    virtual bool uninitialized() const;
    virtual bool is_surdef() const;
    virtual bool is_const() const;

    virtual bool get_val( Type *type, void *data ) const;
    virtual operator BoolOpSeq();
    virtual int op_num() const; ///< only for Op<>

    virtual bool referenced_more_than_one_time() const;

    // codegen
    virtual void get_constraints();
    virtual void update_when( const BoolOpSeq &cond );
    virtual void write( Codegen_C *cc );
    // virtual void add_break_and_continue_internal( CC_SeqItemBlock **b );
    virtual bool will_write_code() const;
    virtual bool need_a_register();
    virtual void codegen_simplification( Vec<Expr> &created, Vec<Expr> &out );

    void add_same_out( int src_ninp, Inst *dst_inst, int dst_ninp, int level );
    void add_diff_out( int src_ninp, Inst *dst_inst, int dst_ninp, int level );
    int  set_inp_reg( int ninp, CppOutReg *reg ); /// -1 -> impossible, 0 -> no change, 1 -> ok with change
    CppOutReg *get_inp_reg( int ninp );

    // display
    static int display_graph( Vec<Expr> outputs, const char *filename = ".res" );
    virtual void write_graph_rec( Vec<Inst *> &ext_buf, Stream &os );
    virtual void write_sub_graph_rec( Stream &os );
    virtual int ext_disp_size() const;

    virtual Expr _simp_repl_bits( Expr off, Expr val );
    virtual Expr _simp_slice( Type *dst, Expr off );
    virtual void _mk_store_dep( Inst *dst );

    Vec<Expr>             inp;
    Vec<Expr>             ext;
    Vec<Expr>             dep;
    mutable Vec<Parent>   par; ///< parents
    mutable Inst         *ext_par;
    int                   flags;

    // codegen
    BoolOpSeq            *when; ///< used for code generation (to know when needed)
    Inst                 *next_sched;
    Inst                 *prev_sched;
    CppOutReg            *out_reg;
    Vec<CppOutReg *>      inp_reg;
    bool                  new_reg;
    std::map<PortConstraint,int> same_out; ///< instructions that must have == out_reg than this. int = COMPULSORY or less
    std::map<PortConstraint,int> diff_out; ///< instructions that must have != out_reg than this. int = COMPULSORY or less
    std::set<CppOutReg *> reg_to_avoid; ///< could be replaced by a reg assignation date

    static  PI64          cur_op_id; ///<
    mutable PI64          op_id_vis; ///<
    mutable PI64          op_id;     ///< operation id (every new operation on the graph begins with ++current_MO_op_id and one can compare op_id with cur_op_id to see if operation on this node has been done or not).
    mutable void         *op_mp;     ///< result of current operations
    mutable int           cpt_use;
};

#endif // INST_H
