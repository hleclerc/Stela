#ifndef BASETYPE_H
#define BASETYPE_H

#include "../System/Stream.h"

/**
*/
class BaseType {
public:
    virtual void write_to_stream( Stream &os, const PI8 *data ) const = 0;
    virtual void write_to_stream( Stream &os ) const = 0;
    virtual int size_in_bytes() const = 0;
    virtual int size_in_bits() const = 0;

    // operations
    #define DECL_OP( OP ) virtual void OP( PI8 *res, const PI8 *da, const PI8 *db ) const = 0;
    #include "DeclOpBinary.h"
    #undef DECL_OP

    #define DECL_OP( OP ) virtual void OP( PI8 *res, const PI8 *da ) const = 0;
    #include "DeclOpUnary.h"
    #undef DECL_OP

    //
    virtual bool conv( PI8 *res, const BaseType *ta, const PI8 *da ) const = 0;
};

#define DECL_BT( T ) extern const BaseType *bt_##T;
#include "DeclArytTypes.h"
#undef DECL_BT

#endif // BASETYPE_H
