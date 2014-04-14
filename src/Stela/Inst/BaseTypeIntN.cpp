#include "../System/Assert.h"
#include "BaseTypeIntN.h"

BaseTypeIntN::BaseTypeIntN( int size ) : size( size ) {}

void BaseTypeIntN::write_c_definition( Stream &os, String reg, const PI8 *data, const PI8 *knwn, int nsp ) const {
    for( int i = 0; i < ( size + 7 ) / 8; ++i ) {
        if ( knwn[ i ] ) {
            for( int i = 0; i < nsp; ++i ) os << ' ';
            os << reg << ".data[ " << i << " ] = " << (int)data[ i ];
            os << ";\n";
        }
    }
}

void BaseTypeIntN::write_to_stream( Stream &os, const PI8 *data ) const { os << "..."; }
void BaseTypeIntN::write_to_stream( Stream &os ) const { os << "IN" << size; }
void BaseTypeIntN::write_c_decl( Stream &os ) const { os << "struct IN" << size << " { char data[ " << ( size + 7 ) / 8 << " ]; };\n"; }
int  BaseTypeIntN::size_in_bytes() const { return ( size_in_bits() + 7 ) / 8; }
int  BaseTypeIntN::size_in_bits() const { return size; }
int  BaseTypeIntN::is_signed() const { return true; }
bool BaseTypeIntN::c_type() const { return false; }
int  BaseTypeIntN::is_fp() const { return false; }

#define DECL_IR_TOK( OP ) void BaseTypeIntN::op_##OP( PI8 *res, const PI8 *da, const PI8 *db ) const { TODO; }
#include "../Ir/Decl_BinaryOperations.h"
#undef DECL_IR_TOK

#define DECL_IR_TOK( OP ) void BaseTypeIntN::op_##OP( PI8 *res, const PI8 *da ) const { TODO; }
#include "../Ir/Decl_UnaryOperations.h"
#undef DECL_IR_TOK

bool BaseTypeIntN::conv( PI8 *res, const BaseType *ta, const PI8 *da ) const { TODO; return false; }
