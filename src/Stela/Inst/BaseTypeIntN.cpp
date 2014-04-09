#include "../System/Assert.h"
#include "BaseTypeIntN.h"

BaseTypeIntN::BaseTypeIntN( int size, String name ) : size( size ), name( name ) {}

void BaseTypeIntN::write_to_stream( Stream &os, const PI8 *data ) const { os << "666"; }
void BaseTypeIntN::write_to_stream( Stream &os ) const { os << name; }
int BaseTypeIntN::size_in_bytes() const { return ( size_in_bits() + 7 ) / 8; }
int BaseTypeIntN::size_in_bits() const { return size; }
int BaseTypeIntN::is_signed() const { return true; }
int BaseTypeIntN::is_fp() const { return false; }

#define DECL_IR_TOK( OP ) void BaseTypeIntN::op_##OP( PI8 *res, const PI8 *da, const PI8 *db ) const { TODO; }
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( OP ) void BaseTypeIntN::op_##OP( PI8 *res, const PI8 *da ) const { TODO; }
#include "../Ir/Decl_UnaryOperations.h"
#undef DECL_IR_TOK

bool BaseTypeIntN::conv( PI8 *res, const BaseType *ta, const PI8 *da ) const { TODO; return false; }
