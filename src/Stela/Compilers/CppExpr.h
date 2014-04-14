#ifndef CPPEXPR_H
#define CPPEXPR_H

#include "../System/Stream.h"
class CppInst;

/**
*/
class CppExpr {
public:
    CppExpr( CppInst *inst = 0, int nout = 0 ) : inst( inst ), nout( nout ) {}
    void write_to_stream( Stream &os ) const;

    bool operator==( const CppExpr &e ) const { return inst == e.inst and nout == e.nout; }
    bool operator!=( const CppExpr &e ) const { return inst != e.inst or  nout != e.nout; }
    bool operator< ( const CppExpr &e ) const { return inst != e.inst ? inst < e.inst : nout < e.nout; }

    bool conv( int &val ) const;

    CppInst *inst;
    int      nout;
};

#endif // CPPEXPR_H
