#ifndef CPPCOMPILER_H
#define CPPCOMPILER_H

#include "../System/SplittedVec.h"
#include "../System/StreamSep.h"
#include "../Inst/Expr.h"
#include "CppInst.h"
#include <sstream>
#include <set>

/**
*/
class CppCompiler {
public:
    CppCompiler();

    CppCompiler &operator<<( ConstPtr<Inst> inst );
    void exec();

    String cpp_filename;
    bool disp_inst_graph, disp_inst_graph_wo_phi;

protected:
    friend class CppInst;
    friend class PhiToIf;

    void compile();
    void output_code_for( Vec<CppInst *> &res );

    CppInst *make_cpp_graph( const Inst *inst, bool force_clone = false );
    void while_precomputations( CppInst *while_inst );
    void while_precomputations_mark_rec( CppInst *inst, CppInst *winp, CppInst *while_inst );
    void while_precomputations_fact_rec( CppInst *inst, CppInst *winp, CppInst *while_inst );
    void get_front_rec( Vec<CppInst *> &front, CppInst *inst );
    void write_inst( CppInst *inst );

    int get_free_reg( const BaseType *bt );
    void add_include( String name );

    // input data
    Vec<ConstPtr<Inst> >    outputs;

    // intermediate data
    SplittedVec<CppInst,64> inst_list;
    SplittedVec<PI8,64>     addd_list; ///< additionnal_data list
    std::set<String>        includes;
    int                     nb_regs;

    // output
    StreamSepMaker<std::ostringstream> on;
    std::ostringstream os;
};

#endif // CPPCOMPILER_H
