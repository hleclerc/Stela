#include "Type.h"
#include "Conv.h"
#include "Cst.h"
#include "Ip.h"

/**
*/
class Conv : public Inst {
public:
    Conv( Type *dst, Type *src ) : dst( dst ), src( src ) {}
    virtual void write_dot( Stream &os ) const {
        os << "Conv[" << *dst << "," << *src << "]";
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new Conv( dst, src );
    }
    virtual int size() const {
        return dst->size();
    }

    Type *dst;
    Type *src;
};

template<class TS>
Expr _conv( Type *dst, const TS &val ) {
    #define DECL_BT( TR ) if ( dst == &ip->type_##TR ) return cst( TR( val ) );
    #include "DeclArytTypes.h"
    #undef DECL_BT
    ERROR( "weird" );
    return Expr();
}

Expr conv( Type *dst, Type *src, Expr val ) {
    if ( dst == src )
        return val;
    PI8 d[ src->size_in_bytes() ];
    if ( dst->_aryth and src->_aryth and val->get_val( d, src->size() ) ) {
        #define DECL_BT( TR ) if ( src == &ip->type_##TR ) return _conv( dst, *reinterpret_cast<TR *>( d ) );
        #include "DeclArytTypes.h"
        #undef DECL_BT
    }

    Conv *res = new Conv( dst, src );
    res->add_inp( val );
    return res;
}
