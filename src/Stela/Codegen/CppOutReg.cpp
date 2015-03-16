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

#define GN( iter ) reinterpret_cast<int &>( (*iter)->op_mp )

Inst *CppOutReg::common_provenance_ancestor() {
    for( Inst::ParentBlockIterator iter( provenance[ 0 ] ); iter; ++iter )
        GN( iter ) = 1;

    for( int n = 1; n < provenance.size(); ++n )
        for( Inst::ParentBlockIterator iter( provenance[ n ] ); iter; ++iter )
            ++GN( iter );

    for( Inst::ParentBlockIterator iter( provenance[ 0 ] ); iter; ++iter )
        if ( GN( iter ) == provenance.size() )
            return *iter;

    return 0;
}



