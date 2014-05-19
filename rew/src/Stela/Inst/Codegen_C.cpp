#include "InstInfo_C.h"
#include "Codegen_C.h"
#include "Ip.h"

Codegen_C::Codegen_C() : on( &main_os ), os( &main_os ) {
    nb_regs = 0;
}

void Codegen_C::write_to( Stream &out ) {
    on.nsp = 4;
    make_code();

    out << "int main() {\n";
    out << main_os.str();
    out << "}\n";
}

void Codegen_C::exec() {
    TODO;
}

static Expr cloned( const Expr &val, Vec<Expr> &created ) {
    val->clone( created );
    return reinterpret_cast<Inst *>( val->op_mp );
}


void Codegen_C::make_code() {
    // clone (-> out)
    ++Inst::cur_op_id;
    Vec<Expr> out, created;
    for( Expr inst : fresh )
        out << cloned( inst, created );
    Expr inst_false = cloned( ip->cst_false, created );
    Expr inst_true  = cloned( ip->cst_true , created );

    // op_mp = inst_info
    SplittedVec<InstInfo_C,16> inst_info_list;
    for( Expr inst : created )
        inst->op_mp = inst_info_list.push_back( inst_false );

    // update IIC(  )->when
    for( Expr inst : out )
        inst->_update_when_C( inst_true );

    for( Expr inst : created )
        std::cout << inst << " when " << IIC( inst )->when << "\n";

    Inst::display_graph( out );
}


