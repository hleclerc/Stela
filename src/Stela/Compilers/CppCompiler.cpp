#include "../Interpreter/Interpreter.h"
#include "../Inst/BaseType.h"
#include "CppCompiler.h"
#include "PhiToIf.h"
#include <fstream>

CppCompiler::CppCompiler() : on( os ) {
    disp_inst_graph_wo_phi = false;
    disp_inst_graph = false;
    cpp_filename = "out.cpp";
    nb_regs = 0;
    on.nsp = 4;
}

CppCompiler &CppCompiler::operator<<( ConstPtr<Inst> inst ) {
    outputs << inst;
    return *this;
}

static bool all_children_are_done( PI64 op_id, const CppInst *inst ) {
    for( int i = 0; i < inst->inp.size(); ++i )
        if ( inst->inp[ i ].inst->op_id < op_id )
            return false;
    return true;
}

void CppCompiler::exec() {
    // fill os
    compile();

    // headers
    std::ofstream fc( cpp_filename.c_str() );
    for( auto f : includes )
        fc << "#include <" << f << ">\n";

    fc << "int main() {\n";
    fc << os.str();
    fc << "}\n";
}

void CppCompiler::compile() {
    if ( disp_inst_graph )
        Inst::display_graph( outputs );

    // make an alternative graph
    ++Inst::cur_op_id;
    Vec<CppInst *> tmp( Rese(), outputs.size() );
    for( int i = 0; i < outputs.size(); ++i )
        tmp << make_cpp_graph( outputs[ i ].ptr() );

    Vec<CppInst *> res = phi_to_if( tmp, this );
    if ( disp_inst_graph_wo_phi )
        CppInst::display_graph( res );

    // get the leaves
    Vec<CppInst *> front;
    ++CppInst::cur_op_id;
    for( int i = 0; i < res.size(); ++i )
        get_front_rec( front, res[ i ] );

    // sweep the tree, starting from the leaves
    PI64 op_id = ++CppInst::cur_op_id;
    while ( front.size() ) {
        CppInst *inst = front.pop_back();
        inst->update_bt_hints();
        inst->op_id = op_id;

        inst->write_code( this );

        for( int nout = 0; nout < inst->out.size(); ++nout )
            for( CppInst::Out::Parent &p : inst->out[ nout ].parents )
                if ( all_children_are_done( op_id, p.inst ) )
                    front.push_back_unique( p.inst );
    }
}

CppInst *CppCompiler::make_cpp_graph( const Inst *inst, bool force_clone ) {
    if ( inst->op_id == Inst::cur_op_id and force_clone == false )
        return reinterpret_cast<CppInst *>( inst->op_mp );
    inst->op_id = Inst::cur_op_id;

    CppInst *res = inst_list.push_back( inst->inst_id(), inst->out_size() );
    inst->op_mp = res;

    res->additionnal_data = addd_list.get_room_for( inst->sizeof_additionnal_data() );
    inst->copy_additionnal_data_to( res->additionnal_data );

    for( int i = 0; i < inst->inp_size(); ++i ) {
        const Inst *ch = inst->inp_expr( i ).inst.ptr();
        res->add_child( make_cpp_graph( ch, ch->inst_id() == Inst::Id_Cst ) );
    }
    for( int i = 0; i < inst->ext_size(); ++i )
        res->add_child( make_cpp_graph( inst->ext_inst( i ) ) );

    return res;
}

void CppCompiler::get_front_rec( Vec<CppInst *> &front, CppInst *inst ) {
    if ( inst->op_id == CppInst::cur_op_id )
        return;
    inst->op_id = CppInst::cur_op_id;

    if ( int nch = inst->inp.size() ) {
        for( int i = 0; i < nch; ++i )
            get_front_rec( front, inst->inp[ i ].inst );
    } else
        front << inst;
}

void CppCompiler::add_include( String name ) {
    includes.insert( name );
}

int CppCompiler::get_free_reg( const BaseType *bt ) {
    return nb_regs++;
}
