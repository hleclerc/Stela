#include "../Interpreter/Interpreter.h"
#include "../Inst/BaseTypeIntN.h"
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
        if ( inst->inp[ i ].inst and inst->inp[ i ].inst->op_id < op_id )
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

    fc << "#include <stdint.h>\n";

    for( const BaseType *bt : bt_to_decl )
        bt->write_c_decl( fc );

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

    // phi -> if
    Vec<CppInst *> res = phi_to_if( tmp, this );

    // while precomputations
    ++CppInst::cur_op_id;
    Vec<CppInst *> while_insts;
    for( int i = 0; i < res.size(); ++i )
        res[ i ]->get_insts_rec( while_insts, Inst::Id_WhileInst );

    for( int i = while_insts.size() - 1; i >= 0; --i )
        while_precomputations( while_insts[ i ] );

    //
    update_bt_hint( res );

    // display
    if ( disp_inst_graph_wo_phi )
        CppInst::display_graph( res );

    //
    output_code_for( res );
}

void CppCompiler::update_bt_hint( Vec<CppInst *> &res ) {
    Vec<CppInst *> lst;
    ++CppInst::cur_op_id;
    for( int i = 0; i < res.size(); ++i )
        get_sub_insts( lst, res[ i ] );

    for( int i = 0; i < lst.size(); ++i )
        lst[ i ]->bt_hint_propagation();

    //
    for( int i = 0; i < lst.size(); ++i ) {
        if ( lst[ i ]->inst_id == Inst::Id_Cst and not lst[ i ]->out[ 0 ].bt_hint ) {
            int size = *reinterpret_cast<const int *>( lst[ i ]->additionnal_data );
            lst[ i ]->set_out_bt_hint( 0, bt_for_size( size ) );
        }
    }
}

const BaseType *CppCompiler::bt_for_size( int size ) {
    if ( size == 8 ) return bt_PI8;
    //
    if ( bt_map.count( size ) )
        return bt_map[ size ].ptr();
    BaseTypeIntN *res = new BaseTypeIntN( size );
    bt_map[ size ] = res;
    return res;
}

void CppCompiler::get_sub_insts( Vec<CppInst *> &res, CppInst *inst ) {
    if ( inst->op_id == CppInst::cur_op_id )
        return;
    inst->op_id = CppInst::cur_op_id;
    res << inst;

    for( int i = 0; i < inst->inp.size(); ++i )
        get_sub_insts( res, inst->inp[ i ].inst );
    for( int i = 0; i < inst->ext_ds; ++i )
        get_sub_insts( res, inst->ext[ i ] );
}

void CppCompiler::output_code_for( Vec<CppInst *> &res ) {
    // get the leaves
    Vec<CppInst *> front;
    ++CppInst::cur_op_id;
    for( int i = 0; i < res.size(); ++i )
        get_front_rec( front, res[ i ] );

    // sweep the tree, starting from the leaves
    PI64 op_id = ++CppInst::cur_op_id;
    while ( front.size() ) {
        CppInst *inst = front.pop_back();
        inst->op_id = op_id;

        inst->write_code( this );

        for( int i = 0; i < inst->out.size(); ++i ) {
            int n = inst->out[ i ].num;
            if ( n >= 0 )
                to_be_used[ n ] += inst->out[ i ].parents.size();
        }

        for( int i = 0; i < inst->inp.size(); ++i ) {
            CppExpr ch = inst->inp[ i ];
            int n = ch.inst->out[ ch.nout ].num;
            if ( n >= 0 and --to_be_used[ n ] == 0 )
                to_be_used.erase( n );
        }

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
        const Expr &ch = inst->inp_expr( i );
        res->add_child( CppExpr( make_cpp_graph( ch.inst.ptr(), ch.inst->inst_id() == Inst::Id_Cst ), ch.nout ) );
    }
    for( int i = 0; i < inst->ext_size(); ++i )
        res->add_ext( make_cpp_graph( inst->ext_inst( i ) ) );
    res->ext_ds = inst->ext_size_disp();

    return res;
}

void CppCompiler::while_precomputations( CppInst *while_inst ) {
    CppInst *wout = while_inst->ext[ 0 ];
    CppInst *winp = while_inst->ext[ 1 ];

    ++CppInst::cur_op_id;
    while_precomputations_mark_rec( wout, winp, while_inst );
    ++CppInst::cur_op_id;
    while_precomputations_fact_rec( wout, winp, while_inst );
}

void CppCompiler::while_precomputations_mark_rec( CppInst *inst, CppInst *winp, CppInst *while_inst ) {
    if ( inst->op_id == CppInst::cur_op_id )
        return;
    inst->op_id = CppInst::cur_op_id;

    if ( inst == winp ) {
        inst->op_mp = inst;
        return;
    }

    int nch = inst->inp.size();
    bool depends_on_winp = false;
    for( int i = 0; i < nch; ++i ) {
        CppInst *ch = inst->inp[ i ].inst;
        while_precomputations_mark_rec( ch, winp, while_inst );
        depends_on_winp = depends_on_winp or ch->op_mp;
    }

    inst->op_mp = depends_on_winp ? inst : 0;
}

void CppCompiler::while_precomputations_fact_rec( CppInst *inst, CppInst *winp, CppInst *while_inst ) {
    if ( inst->op_id == CppInst::cur_op_id )
        return;
    inst->op_id = CppInst::cur_op_id;

    for( int i = 0, nch = inst->inp.size(); i < nch; ++i ) {
        CppInst *ch = inst->inp[ i ].inst;
        if ( ch->op_mp or ch->inst_id == Inst::Id_Cst ) {
            while_precomputations_fact_rec( ch, winp, while_inst );
        } else {
            for( int n = 0; ; ++n ) {
                // need to create a new entry in if_inst (leading to a new output in ifinp) ?
                if ( n == while_inst->inp.size() ) {
                    while_inst->add_child( inst->inp[ i ] );
                    winp->check_out_size( n + 1 );
                    inst->set_child( i, CppExpr( winp, n ) );
                    break;
                }
                // already in input of if_inst ?
                if ( while_inst->inp[ n ] == inst->inp[ i ] ) {
                    winp->check_out_size( n + 1 );
                    inst->set_child( i, CppExpr( winp, n ) );
                    break;
                }
            }
        }
    }
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
