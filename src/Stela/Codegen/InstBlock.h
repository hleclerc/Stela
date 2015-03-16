#ifndef INSTBLOCK_H
#define INSTBLOCK_H

#include "../Ssa/Inst.h"
#include <map>

/**
*/
class InstBlock : public Inst {
public:
    InstBlock( InstBlock *parent = 0 );
    virtual Expr forced_clone( Vec<Expr> &created ) const;
    virtual void write_dot( Stream &os ) const;
    virtual int op_type() const { return ID_OP_InstBlock; }
    virtual Bool _same_op( Inst *b ) { TODO; return false; }
    virtual bool need_out_reg();

    void operator<<( Inst *inst );

    int  add_use( OutReg *reg );
    void add_decl( OutReg *out_reg );

    virtual void write( Codegen *c );

    InstBlock                      *parent;
    Vec<Expr>                       seq;
    std::map<Type *,Vec<OutReg *> > decl;

    OutReg                         *use_out_reg;
    int                             use_cpt;
};

#endif // INSTBLOCK_H
