#include "CppCompiler.h"

CppCompiler::CppCompiler() {
}

CppCompiler &CppCompiler::operator<<( ConstPtr<Inst> inst ) {
    outputs << inst;
    return *this;
}

void CppCompiler::exec() {
    // get the leaves
    Vec<const Inst *> front;
    ++Inst::cur_op_id;
    for( int i = 0; i < outputs.size(); ++i )
        get_front_rec( front, outputs[ i ].ptr() );

    // sweep the tree, starting from the leae
    ++Inst::cur_op_id;
    while ( front.size() ) {
        const Inst *inst = front.pop_back();
        PRINT( *inst );

//        for( int i = 0; i < item->parents.size(); ++i ) {
//            if ( all_children_are_done( item->parents[ i ] ) ) {
//                item->parents[ i ]->is_in_front = true;
//                if ( best_parent_in_front == 0 or best_parent_in_front->depth < item->parents[ i ]->depth )
//                    best_parent_in_front = item->parents[ i ];
//            }
//        }

    }
}


void CppCompiler::get_front_rec( Vec<const Inst *> &front, const Inst *inst ) {
    if ( inst->op_id == Inst::cur_op_id )
        return;
    inst->op_id = Inst::cur_op_id;

    if ( int nch = inst->inp_size() ) {
        for( int i = 0; i < nch; ++i )
            get_front_rec( front, inst->inp_expr( i ).inst.ptr() );
    } else
        front << inst;

}
