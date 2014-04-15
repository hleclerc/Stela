#ifndef CPPCOMPILER_H
#define CPPCOMPILER_H

#include "../System/SplittedVec.h"
#include "../System/StreamSep.h"
#include "../System/AutoPtr.h"
#include "../Inst/BaseType.h"
#include "../Inst/Expr.h"
#include "CppInst.h"
#include <sstream>
#include <set>
#include <map>

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
    // void get_pointed_values( Vec<CppExpr> &pointed_values, CppInst *inst );
    void get_front_rec( Vec<CppInst *> &front, CppInst *inst );
    void get_sub_insts( Vec<CppInst *> &res, CppInst *inst );
    const BaseType *bt_for_size( int size );
    void update_bt_hint( Vec<CppInst *> &res );
    void dec_to_be_used_rec( CppExpr ch );
    void write_inst( CppInst *inst );

    int get_free_reg( const BaseType *bt );
    void add_include( String name );

    // input data
    Vec<ConstPtr<Inst> >             outputs;

    // intermediate data
    SplittedVec<CppInst,64>          inst_list;
    SplittedVec<PI8,64>              addd_list; ///< additionnal_data list
    std::set<String>                 includes;

    std::map<int,int>                to_be_used; ///< nb of time a register have to be used

    std::map<int,AutoPtr<BaseType> > bt_map; ///< size vs bt
    std::set<const BaseType *>       bt_to_decl;
    Vec<const BaseType *>            reg_types;

    // output
    StreamSepMaker<std::ostringstream> on;
    std::ostringstream os;
};

#endif // CPPCOMPILER_H
