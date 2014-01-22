#ifndef EXPR_H
#define EXPR_H

#include <string.h>
#include "Inst.h"

/**
*/
class Expr {
public:
    typedef Vec<Inst::Out::Item,-1,1> TPar;

    Expr( Ptr<Inst> inst, int nout = 0 );
    Expr();

    const PI8 *cst_data() const;
    const PI8 *cst_data_ValAt( int off = 0 ) const;
    void write_to_stream( Stream &os ) const;
    operator bool() const { return inst; }
    int size_in_bits() const;
    int size_in_bytes() const;
    const BaseType *out_bt() const;

    const TPar &parents() const { return inst->out_expr( nout ).parents; }
    TPar &parents() { return inst->out_expr( nout ).parents; }

    bool operator==( const Expr &e ) const { return inst == e.inst and nout == e.nout; }
    bool operator!=( const Expr &e ) const { return inst != e.inst or  nout != e.nout; }
    bool operator< ( const Expr &e ) const { return inst != e.inst ? inst < e.inst : nout < e.nout; }

    template<class T>
    bool basic_conv( T &val ) const {
        if ( const PI8 *data = cst_data() ) {
            if ( size_in_bytes() >= sizeof( T ) ) {
                memcpy( &val, data, sizeof( T ) );
                return true;
            }
        }
        return false;
    }


    Ptr<Inst> inst;
    int       nout; ///< num output of inst to use
};

#endif // EXPR_H
