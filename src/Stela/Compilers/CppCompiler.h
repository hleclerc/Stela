#ifndef CPPCOMPILER_H
#define CPPCOMPILER_H

#include "../System/SplittedVec.h"
#include "../System/StreamSep.h"
#include "../Inst/BaseType.h"
#include "../Inst/Expr.h"
#include "CppInstInfo.h"
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
    friend class CppInstCompiler;
    friend class CppInstInfo;

    void get_front_rec( Vec<const Inst *> &front, const Inst *inst );
    void get_base_type_rec( const Inst *inst );

    void compile();

    int get_free_reg( const BaseType *bt );
    void add_include( String name );

    SplittedVec<CppInstInfo,64> info_it;
    Vec<ConstPtr<Inst> >        outputs;
    std::set<String>            includes;
    int                         nb_regs;

    StreamSepMaker<std::ostringstream> on;
    std::ostringstream os;
};

#endif // CPPCOMPILER_H
