#ifndef BASETYPEINTN_H
#define BASETYPEINTN_H

#include "../System/Stream.h"
#include "BaseType.h"

class BaseTypeIntN : public BaseType {
public:
    BaseTypeIntN( int size, String name );

    virtual void write_to_stream( Stream &os, const PI8 *data ) const;
    virtual void write_to_stream( Stream &os ) const;
    virtual int size_in_bytes() const;
    virtual int size_in_bits() const;
    virtual int is_signed() const;
    virtual int is_fp() const;

    #define DECL_IR_TOK( OP ) virtual void op_##OP( PI8 *res, const PI8 *da, const PI8 *db ) const;
    #include "../Ir/Decl_BinaryOperations.h"
    #undef DECL_IR_TOK

    #define DECL_IR_TOK( OP ) virtual void op_##OP( PI8 *res, const PI8 *da ) const;
    #include "../Ir/Decl_UnaryOperations.h"
    #undef DECL_IR_TOK

    virtual bool conv( PI8 *res, const BaseType *ta, const PI8 *da ) const;


protected:
    int size;
    String name;
};

#endif // BASETYPEINTN_H
