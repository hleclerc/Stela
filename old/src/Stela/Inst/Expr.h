#ifndef EXPR_H
#define EXPR_H

#include "../System/Memcpy.h"
#include "SizeInBits.h"
#include <string.h>
#include "Inst.h"

/**
*/
class Expr {
public:
    typedef Vec<Inst::Out::Parent,-1,1> TPar;

    Expr( Ptr<Inst> inst, int nout = 0 );
    Expr();

    const PI8 *cst_data( int beg, int end ) const;
    const PI8 *cst_data() const;

    const PI8 *vat_data( int beg, int end ) const;
    const PI8 *vat_data() const;

    void write_to_stream( Stream &os ) const;
    operator bool() const { return inst; }
    //const BaseType *out_bt() const;
    int size_in_bytes() const;
    int size_in_bits() const;

    const TPar &parents() const { return inst->out_expr( nout ).parents; }
    TPar &parents() { return inst->out_expr( nout ).parents; }

    bool operator==( const Expr &e ) const { return inst == e.inst and nout == e.nout; }
    bool operator!=( const Expr &e ) const { return inst != e.inst or  nout != e.nout; }
    bool operator< ( const Expr &e ) const { return inst != e.inst ? inst < e.inst : nout < e.nout; }

    template<class T>
    bool get_val( T &val, int offset = 0 ) const {
        if ( const PI8 *data = cst_data( offset, offset + SizeInBits<T>::res ) ) {
            memcpy_bit( (PI8 *)&val, data, SizeInBits<T>::res );
            return true;
        }
        return false;
    }

    template<class T>
    bool get_vat( T &val, int offset = 0 ) const {
        if ( const PI8 *data = vat_data( offset, offset + 8 * sizeof( T ) ) ) {
            memcpy_bit( (PI8 *)&val, data, SizeInBits<T>::res );
            return true;
        }
        return false;
    }

    Ptr<Inst> inst;
    int       nout; ///< num output of inst to use
};

#endif // EXPR_H
