#ifndef CPPEXPR_H
#define CPPEXPR_H

class CppInst;

/**
*/
class CppExpr {
public:
    CppExpr( CppInst *inst = 0, int nout = 0 ) : inst( inst ), nout( nout ) {}


    CppInst *inst;
    int      nout;
};

#endif // CPPEXPR_H
