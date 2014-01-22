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
        return sizeof( T );
    }
    virtual int size_in_bits() const {
        return 8 * sizeof( T );
    }

    virtual void add( PI8 *res, const PI8 *da, const PI8 *db ) const { *reinterpret_cast<T *>( res ) = *reinterpret_cast<const T *>( da ) + *reinterpret_cast<const T *>( da ); }
    virtual void and_op( PI8 *res, const PI8 *da, const PI8 *db ) const { *reinterpret_cast<T *>( res ) = *reinterpret_cast<const T *>( da ) & *reinterpret_cast<const T *>( da ); }
    virtual void not_op( PI8 *res, const PI8 *da ) const { *reinterpret_cast<T *>( res ) = ~ *reinterpret_cast<const T *>( da ); }

    const char *name;
};

#define DECL_BT( T ) BaseType_<T> ibt_##T( #T ); const BaseType *bt_##T = &ibt_##T;
#include "DeclArytTypes.h"
#undef DECL_BT
