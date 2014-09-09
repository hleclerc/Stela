#ifndef CODEGEN_C_H
#define CODEGEN_C_H

#include "../System/SplittedVec.h"
#include "../System/StreamSep.h"
#include "CppOutReg.h"
#include "Codegen.h"
#include <set>
#include <map>

/**
*/
class Codegen_C : public Codegen {
public:
    Codegen_C();

    virtual void write_to( Stream &os );
    virtual void exec();

    void add_include( String name );
    void add_prel( String data );
    void add_type( Type *type );

    CppOutReg *new_out_reg( Type *type );
    String type_to_str( Type *type );
    void write_out( Expr expr );

    StreamSepMaker<std::ostringstream> on;
    std::ostringstream main_os;
    std::ostringstream *os;
    std::set<String> includes;
    Vec<String> preliminaries;
    std::set<Type *> types;

    bool disp_inst_graph;
    bool disp_inst_graph_wo_phi;

protected:
    void make_code();
    Inst *scheduling( Vec<Expr> out ); ///< return the first inst to be scheduled

    SplittedVec<CppOutReg,32> out_regs;
    std::map<int,String>      spec_types;
};

#endif // CODEGEN_C_H
