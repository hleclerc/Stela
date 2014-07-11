#include "../Inst/Type.h"

#include "CC_SeqItemBlock.h"
#include "CppOutReg.h"
#include "Codegen_C.h"

CppOutReg::CppOutReg( Type *type, int num ) : type( type ), num( num ) {
}

Stream &CppOutReg::write( Codegen_C *cc, bool new_reg ) {
    cc->add_type( type );
    if ( new_reg )
        *cc->os << *type << " ";
    return *cc->os << "R" << num;
}

CC_SeqItemBlock *CppOutReg::common_provenance_ancestor() {
    for( CC_SeqItemBlock *b : provenance )
        for( ; b; b = b->parent_block )
            b->n = 0;

    for( CC_SeqItemBlock *b : provenance )
        for( ; b; b = b->parent_block )
            ++b->n;

    for( CC_SeqItemBlock *b = provenance[ 0 ]; b; b = b->parent_block )
        if ( b->n == provenance.size() )
            return b;
    return 0;
}



