#include "../System/Math.h"
#include "SizeInBits.h"
#include "BaseType.h"
#include <string.h>

BaseType::~BaseType() {
}

template<class T> static T _or    ( const T &a, const T &b ) { return a | b; }
template<class T> static T _and   ( const T &a, const T &b ) { return a & b; }
template<class T> static T _add   ( const T &a, const T &b ) { return a + b; }
template<class T> static T _sub   ( const T &a, const T &b ) { return a - b; }
template<class T> static T _mul   ( const T &a, const T &b ) { return a * b; }
template<class T> static T _div   ( const T &a, const T &b ) { return a / b; }
template<class T> static T _mod   ( const T &a, const T &b ) { return a % b; }
template<class T> static T _pow   ( const T &a, const T &b ) { return pow( a, b ); }
template<class T> static T _equ   ( const T &a, const T &b ) { return a == b; }
template<class T> static T _neq   ( const T &a, const T &b ) { return a != b; }
template<class T> static T _sup   ( const T &a, const T &b ) { return a >  b; }
template<class T> static T _inf   ( const T &a, const T &b ) { return a <  b; }
template<class T> static T _inf_eq( const T &a, const T &b ) { return a <= b; }
template<class T> static T _sup_eq( const T &a, const T &b ) { return a >= b; }

static Bool _not ( const Bool &a ) { return not a; }
template<class T> static T _not ( const T &a ) { return ~ a; }
template<class T> static T _log ( const T &a ) { return log( a ); }
template<class T> static T _ceil( const T &a ) { return ceil( a ); }

template<class T> struct IsSigned { enum { res = true }; };
template<> struct IsSigned<PI8 > { enum { res = false }; };
template<> struct IsSigned<PI16> { enum { res = false }; };
template<> struct IsSigned<PI32> { enum { res = false }; };
template<> struct IsSigned<PI64> { enum { res = false }; };

template<class T> struct IsFp { enum { res = false }; };
template<> struct IsFp<FP32> { enum { res = true }; };
template<> struct IsFp<FP64> { enum { res = true }; };
template<> struct IsFp<FP80> { enum { res = true }; };

template<class T> void _disp( Stream &os, const T &data ) { os << data; }
static void _disp( Stream &os, const PI8 &data ) { os << (int)data; }
static void _disp( Stream &os, const SI8 &data ) { os << (int)data; }

namespace {
const char *cpp_name( S<FP32> ) { return "float"; }
const char *cpp_name( S<FP64> ) { return "double"; }
const char *cpp_name( S<FP80> ) { return "long double"; }
const char *cpp_name( S<Bool> ) { return "bool"; }
const char *cpp_name( S<SI8 > ) { return "int8_t"; }
const char *cpp_name( S<SI16> ) { return "int16_t"; }
const char *cpp_name( S<SI32> ) { return "int32_t"; }
const char *cpp_name( S<SI64> ) { return "int64_t"; }
const char *cpp_name( S<PI8 > ) { return "uint8_t"; }
const char *cpp_name( S<PI16> ) { return "uint16_t"; }
const char *cpp_name( S<PI32> ) { return "uint32_t"; }
const char *cpp_name( S<PI64> ) { return "uint64_t"; }
}

/**
*/
template<class T>
struct BaseType_ : BaseType {
    BaseType_( const char *name ) : name( name ) {
    }
    virtual void write_c_definition( Stream &os, String reg, const PI8 *data, const PI8 *knwn, int nsp ) const {
        if ( knwn == 0 or *knwn /*TODO: test all the bits*/ ) {
            for( int i = 0; i < nsp; ++i ) os << ' ';
            _disp( os << reg << " = ", *reinterpret_cast<const T *>( data ) );
            os << ";\n";
        }
    }
    virtual void write_to_stream( Stream &os, const PI8 *data ) const {
        _disp( os, *reinterpret_cast<const T *>( data ) );
    }
    virtual void write_to_stream( Stream &os ) const {
        os << name;
    }
    virtual void write_c_decl( Stream &os ) const {
        os << "typedef " << cpp_name( S<T>() ) << " " << name << ";\n";
    }
    virtual int size_in_bytes() const {
        return ( SizeInBits<T>::res + 7 ) / 8;
    }
    virtual int size_in_bits() const {
        return SizeInBits<T>::res;
    }
    virtual int is_signed() const {
        return IsSigned<T>::res;
    }
    virtual bool c_type() const {
        return true;
    }
    virtual int is_fp() const {
        return IsFp<T>::res;
    }

    #define DECL_IR_TOK( OP ) virtual void op_##OP( PI8 *res, const PI8 *da, const PI8 *db ) const  { *reinterpret_cast<T *>( res ) = _##OP( *reinterpret_cast<const T *>( da ), *reinterpret_cast<const T *>( db ) ); }
    #include "../Ir/Decl_BinaryOperations.h"
    #undef DECL_IR_TOK

    #define DECL_IR_TOK( OP ) virtual void op_##OP( PI8 *res, const PI8 *da ) const  { *reinterpret_cast<T *>( res ) = _##OP( *reinterpret_cast<const T *>( da ) ); }
    #include "../Ir/Decl_UnaryOperations.h"
    #undef DECL_IR_TOK

    virtual bool conv( PI8 *res, const BaseType *ta, const PI8 *da ) const {
        #define DECL_BT( U ) if ( ta == bt_##U ) return ::conv( *reinterpret_cast<T *>( res ), *reinterpret_cast<const U *>( da ) );
        #include "DeclArytTypes.h"
        #undef DECL_BT
        if ( ta == bt_Void ) {
            *reinterpret_cast<T *>( res ) = 0;
            return true;
        }
        return false;

    }

    const char *name;
};

#define DECL_BT( T ) BaseType_<T> ibt_##T( #T ); const BaseType *bt_##T = &ibt_##T;
#include "DeclArytTypes.h"
#undef DECL_BT

struct BaseType_Void : BaseType {
    BaseType_Void( const char *name ) : name( name ) { }
    virtual void write_c_definition( Stream &os, String reg, const PI8 *data, const PI8 *knwn, int nsp ) const {}
    virtual void write_to_stream( Stream &os, const PI8 *data ) const { os << "void"; }
    virtual void write_to_stream( Stream &os ) const { os << name; }
    virtual void write_c_decl( Stream &os ) const { os << "typedef void        Void;\n"; }
    virtual int  size_in_bytes() const { return 0; }
    virtual int  size_in_bits() const { return 0; }
    virtual int  is_signed() const { return false; }
    virtual bool c_type() const { return true; }
    virtual int  is_fp() const { return false; }

    #define DECL_IR_TOK( OP ) virtual void op_##OP( PI8 *res, const PI8 *da, const PI8 *db ) const  {}
    #include "../Ir/Decl_BinaryOperations.h"
    #undef DECL_IR_TOK

    #define DECL_IR_TOK( OP ) virtual void op_##OP( PI8 *res, const PI8 *da ) const  {}
    #include "../Ir/Decl_UnaryOperations.h"
    #undef DECL_IR_TOK

    virtual bool conv( PI8 *res, const BaseType *ta, const PI8 *da ) const {
        return ta == bt_Void;
    }

    const char *name;
};


BaseType_Void ibt_Void( "Void" ); const BaseType *bt_Void = &ibt_Void;

const BaseType *get_bt( int size_in_bits, bool is_signed, bool is_fp ) {
    #define DECL_BT( T ) if ( IsSigned<T>::res == is_signed and IsFp<T>::res == is_fp and SizeInBits<T>::res >= size_in_bits ) return bt_##T;
    #include "DeclArytTypes.h"
    #undef DECL_BT
    return 0;
}

const BaseType *type_promote( const BaseType *ta, const BaseType *tb ) {
    return get_bt( std::max( ta->size_in_bits(), tb->size_in_bits() ),
                   std::max( ta->is_signed(), tb->is_signed() ),
                   std::max( ta->is_fp(), tb->is_fp() )
   );
}
