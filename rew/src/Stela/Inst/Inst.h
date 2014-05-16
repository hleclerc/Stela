#ifndef INST_H
#define INST_H

#include "../System/Vec.h"
#include <string.h>
#include "Expr.h"
class CodeGen_C;
class Type;

/**
*/
class Inst {
public:
    enum {
        TPAR_DEP = -1,
        TPAR_CND = -2
    };

    struct Parent {
        bool operator==( const Parent &p ) { return inst == p.inst and ninp == p.ninp; }
        Inst *inst;
        int   ninp; ///< input number (or TPAR_...)
    };


    Inst();
    virtual ~Inst();

    virtual void write_to_stream( Stream &os ) const;
    virtual void write_dot( Stream &os ) const = 0;

    void add_dep( const Expr &val );
    void set_cnd( const Expr &val );
    void add_inp( const Expr &val );
    void mod_inp( const Expr &val, int num );

    virtual void clone( Vec<Expr> &created ) const;
    virtual Expr forced_clone( Vec<Expr> &created ) const = 0;

    template<class T>
    bool get_val( T &res ) { if ( const PI8 *ptr = data_ptr() ) { int sb = ( size() + 7 ) / 8; if ( sizeof( res ) <= sb ) { memcpy( &res, ptr, sizeof( res ) ); return true; } } return false; }

    virtual int size() const = 0;
    virtual const PI8 *data_ptr( int offset = 0 ) const;

    virtual bool ok_if( Expr cond );
    virtual bool ko_if( Expr cond );
    virtual Expr val_if( Expr cond );
    virtual Expr rval_if( Expr val );

    virtual bool is_a_pointer() const;
    virtual Type *out_type_proposition( CodeGen_C *cc ) const;
    virtual Type *inp_type_proposition( CodeGen_C *cc, int ninp ) const;

    virtual Expr _simplified();
    virtual Expr _get_val() const;
    virtual void _set_val( Expr val );
    virtual Expr _at( int len );

    Vec<Expr>           inp; ///< inputs
    Vec<Expr>           dep; ///< dependencies
    Vec<Expr>           ext; ///< WhileOut, ...
    Expr                cnd; ///< condition
    mutable Vec<Parent> par; ///< parents

    Inst               *ext_par;

    static  PI64        cur_op_id; ///<
    mutable PI64        op_id_vis; ///<
    mutable PI64        op_id;     ///< operation id (every new operation on the graph begins with ++current_MO_op_id and one can compare op_id with cur_op_id to see if operation on this node has been done or not).
    mutable void       *op_mp;     ///< result of current operations
    mutable int         cpt_use;
};

// inlined Expr methods
#include "_Expr_defs.h"

#endif // INST_H
