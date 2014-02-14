#include "../Interpreter/Interpreter.h"
#include "../Inst/BaseType.h"
#include "CppInstCompiler.h"
#include "CppCompiler.h"
#include <fstream>

#define INFO( inst ) reinterpret_cast<CppInstInfo *>( (inst)->op_mp )

CppCompiler::CppCompiler() : on( os ) {
    disp_inst_graph = false;
    cpp_filename = "out.cpp";
    nb_regs = 0;
    on.nsp = 4;
}

CppCompiler &CppCompiler::operator<<( ConstPtr<Inst> inst ) {
    outputs << inst;
    return *this;
}

static bool all_children_are_done( PI64 op_id, const Inst *inst ) {
    for( int i = 0; i < inst->inp_size(); ++i )
        if ( inst->inp_expr( i ).inst->op_id < op_id )
            return false;
    return true;
}

void CppCompiler::exec() {
    compile();

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

    // get the leaves and inst->op_mp to new CppInstInfo instances
    Vec<const Inst *> front;
    ++Inst::cur_op_id;
    for( int i = 0; i < outputs.size(); ++i )
        get_front_rec( front, outputs[ i ].ptr() );

    // get BaseType
    ++Inst::cur_op_id;
    for( int i = 0; i < outputs.size(); ++i )
        get_base_type_rec( outputs[ i ].ptr() );

    // sweep the tree, starting from the leaves
    PI64 op_id = ++Inst::cur_op_id;
    CppInstCompiler cic( this );
    while ( front.size() ) {
        const Inst *inst = front.pop_back();
        inst->op_id = op_id;

        inst->apply( cic );

        for( int nout = 0; nout < inst->out_size(); ++nout )
            for( const auto &p : inst->out_expr( nout ).parents )
                if ( all_children_are_done( op_id, p.inst ) )
                    front.push_back_unique( p.inst );
    }
}

void CppCompiler::get_front_rec( Vec<const Inst *> &front, const Inst *inst ) {
    if ( inst->op_id == Inst::cur_op_id )
        return;
    inst->op_id = Inst::cur_op_id;
    inst->op_mp = info_it.push_back( inst );

    if ( int nch = inst->inp_size() ) {
        for( int i = 0; i < nch; ++i )
            get_front_rec( front, inst->inp_expr( i ).inst.ptr() );
    } else
        front << inst;
}

struct CppInstTypeHint : InstVisitor {
    virtual void def( const Inst &inst ) { hint = 0; }
    virtual void syscall( const Inst &inst ) { hint = ip->bt_ST; }
    const BaseType *hint;
    int ninp;
};

void CppCompiler::get_base_type_rec( const Inst *inst ) {
    if ( inst->op_id == Inst::cur_op_id )
        return;
    inst->op_id = Inst::cur_op_id;

    for( int no = 0; no < inst->out_size(); ++no ) {
        if ( const BaseType *bt = inst->out_bt( no ) ) {
            INFO( inst )->out[ no ].type = bt;
            continue;
        }
        // else look in parents if there is a particular utilisation
        int ps = inst->out_expr( no ).parents.size();
        CppInstTypeHint citi;
        for( int np = 0; np < ps; ++np ) {
            const Inst::Out::Parent &p = inst->out_expr( no ).parents[ np ];
            citi.ninp = p.ninp;
            p.inst->apply( citi );
            if ( citi.hint )
                break;
        }
        if ( citi.hint ) {
            INFO( inst )->out[ no ].type = citi.hint;
            continue;
        }
        // else, make a "fake" bt
        int s = inst->size_in_bits( no );
        switch ( s ) {
        case (  0 ): break;
        case (  1 ): INFO( inst )->out[ no ].type = bt_Bool; break;
        case (  8 ): INFO( inst )->out[ no ].type = bt_PI8 ; break;
        case ( 16 ): INFO( inst )->out[ no ].type = bt_PI16; break;
        case ( 32 ): INFO( inst )->out[ no ].type = bt_PI32; break;
        case ( 64 ): INFO( inst )->out[ no ].type = bt_PI64; break;
        default:
            PRINT( s );
            TODO; // make an internal "fake" bt
        }
    }

    for( int i = 0; i < inst->inp_size(); ++i )
        get_base_type_rec( inst->inp_expr( i ).inst.ptr() );
}


void CppCompiler::add_include( String name ) {
    includes.insert( name );
}

int CppCompiler::get_free_reg( const BaseType *bt ) {
    return nb_regs++;
}
