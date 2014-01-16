#include "BaseType.h"
#include <string.h>

/**
*/
template<class T>
struct BaseType_ : BaseType {
    BaseType_( const char *name ) : name( name ) {}
    virtual void write_to_stream( Stream &os, const PI8 *data ) const {
        os << *reinterpret_cast<const T *>( data );
    }
    virtual void write_to_stream( Stream &os ) const {
        os << name;
    }
    virtual int size_in_bytes() const {
        return sizeof( T );
    }

    const char *name;
};

#define DECL_BT( T ) BaseType_<T> ibt_##T( #T ); const BaseType *bt_##T = &ibt_##T;
#include "DeclArytTypes.h"
#undef DECL_BT
