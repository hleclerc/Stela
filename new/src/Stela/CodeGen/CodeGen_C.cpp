#include "CodeGen_C.h"

CodeGen_C::CodeGen_C() : on( &main_os ), os( &main_os ) {
    on << "int main() {}";
}

void CodeGen_C::write_to( Stream &out ) {
    make_code();
    out << main_os.str();
}

void CodeGen_C::exec() {

}

void CodeGen_C::make_code() {
    // clone -> out
    ++Inst::cur_op_id;
    Vec<Ptr<Inst> > out, created;
    for( ConstPtr<Inst> inst : fresh ) {
        inst->clone( created );
        out << reinterpret_cast<Inst *>( inst->op_mp );
    }

    Inst::display_graph( out );
}
