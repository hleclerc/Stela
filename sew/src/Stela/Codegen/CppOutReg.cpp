#include "../Inst/Type.h"

// #include "CC_SeqItemBlock.h"
// #include "CC_SeqItemExpr.h"
#include "CppOutReg.h"
#include "Codegen_C.h"

CppOutReg::CppOutReg( Type *type, int num ) : type( type ), num( num ) {
}

void CppOutReg::write_to_stream( Stream &os ) const {
    os << "R" << num;
}

Stream &CppOutReg::write( Codegen_C *cc, bool new_reg ) {
    cc->add_type( type );
    if ( new_reg )
        *cc->os << *type << " ";
    return *cc->os << "R" << num;
}

//CC_SeqItemBlock *CppOutReg::common_provenance_ancestor() {
//    for( CC_SeqItemExpr *e : provenance )
//        for( CC_SeqItemBlock *b = e->parent_block; b; b = b->parent_block )
//            b->n = 0;

//    for( CC_SeqItemExpr *e : provenance )
//        for( CC_SeqItemBlock *b = e->parent_block; b; b = b->parent_block )
//            ++b->n;

//    for( CC_SeqItemBlock *b = provenance[ 0 ]->parent_block; b; b = b->parent_block )
//        if ( b->n == provenance.size() )
//            return b;
//    return 0;
//}



