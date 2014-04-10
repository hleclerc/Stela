#ifndef BASETYPE_H
#define BASETYPE_H

#include "../System/Stream.h"
#include "../System/S.h"

/**
*/
class BaseType {
public:
    virtual ~BaseType();
    virtual void write_to_stream( Stream &os, const PI8 *data ) const = 0;
    virtual void write_to_stream( Stream &os ) const = 0;
    virtual int size_in_bytes() const = 0;
    virtual int size_in_bits() const = 0;
    virtual int is_signed() const = 0;
    virtual int is_fp() const = 0;

    // operations
    #define DECL_IR_TOK( OP ) virtual void op_##OP( PI8 *res, const PI8 *da, const PI8 *db ) const = 0;
    #include "../Ir/Decl_BinaryOperations.h"
    #undef DECL_IR_TOK

    #define DECL_IR_TOK( OP ) virtual void op_##OP( PI8 *res, const PI8 *da ) const = 0;
    #include "../Ir/Decl_UnaryOperations.h"
    #undef DECL_IR_TOK

    //
    virtual bool conv( PI8 *res, const BaseType *ta, const PI8 *da ) const = 0;
};

#define DECL_BT( T ) extern const BaseType *bt_##T; inline const BaseType *bt_for_type( S<T> ) { return bt_##T; }
#include "DeclArytTypes.h"
#undef DECL_BT

extern const BaseType *bt_Void;

const BaseType *get_bt( int size_in_bits, bool is_signed, bool is_fp );
const BaseType *type_promote( const BaseType *ta, const BaseType *tb );

#endif // BASETYPE_H
