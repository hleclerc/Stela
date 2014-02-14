#include "../System/Math.h"
#include "SizeInBits.h"
#include "BaseType.h"
#include <string.h>


/**
*/
template<class T>
struct BaseType_ : BaseType {
    BaseType_( const char *name ) : name( name ) {
    }
    virtual void write_to_stream( Stream &os, const PI8 *data ) const {
        os << *reinterpret_cast<const T *>( data );
    }
    virtual void write_to_stream( Stream &os ) const {
        os << name;
    }
    virtual int size_in_bytes() const {
        return ( SizeInBits<T>::res + 7 ) / 8;
    }
    virtual int size_in_bits() const {
        return SizeInBits<T>::res;
    }

    virtual void add( PI8 *res, const PI8 *da, const PI8 *db ) const { *reinterpret_cast<T *>( res ) = *reinterpret_cast<const T *>( da ) + *reinterpret_cast<const T *>( da ); }
    virtual void and_op( PI8 *res, const PI8 *da, const PI8 *db ) const { *reinterpret_cast<T *>( res ) = *reinterpret_cast<const T *>( da ) & *reinterpret_cast<const T *>( da ); }
    virtual void not_op( PI8 *res, const PI8 *da ) const { *reinterpret_cast<T *>( res ) = ~ *reinterpret_cast<const T *>( da ); }

    virtual bool conv( PI8 *res, const BaseType *ta, const PI8 *da ) const {
        #define DECL_BT( U ) if ( ta == bt_##U ) return ::conv( *reinterpret_cast<T *>( res ), *reinterpret_cast<const U *>( da ) );
        #include "DeclArytTypes.h"
        #undef DECL_BT
        return 0;
    }

    const char *name;
};

#define DECL_BT( T ) BaseType_<T> ibt_##T( #T ); const BaseType *bt_##T = &ibt_##T;
#include "DeclArytTypes.h"
#undef DECL_BT
