#include "CppExpr.h"
#include "CppInst.h"

void CppExpr::write_to_stream( Stream &os ) const {
    os << *inst;
    if ( inst->out.size() > 1 )
        os << '(' << nout << ')';
}

bool CppExpr::conv( int &val ) const {
    return inst->conv( val, nout );
}
