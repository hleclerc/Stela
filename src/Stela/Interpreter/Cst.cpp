#include "InstVisitor.h"
#include "Cst.h"
#include <set>

struct CstCmp {
    bool operator()( const Cst *a, const Cst *b ) const {
        if ( a->value == b->value )
            return a->known < b->known;
        return a->value < b->value;
    }
};

static std::set<Cst *,CstCmp> cst_set;


Cst::~Cst() {
    cst_set.erase( this );
}

void Cst::write_to_stream( Stream &os ) const {
    const char *c = "0123456789ABCDEF";
    for( int i = 0; i < value.size(); ++i ) {
        if ( i )
            os << ' ';
        os << c[ value[ i ] >> 4 ] << c[ value[ i ] & 0xF ];
    }
}

void Cst::apply( InstVisitor &visitor ) const {
    visitor( *this );
}

const PI8 *Cst::cst_data( int nout ) const {
    return value.ptr();
}


Expr cst( Vec<PI8> value ) {
    Cst *tmp = new Cst;
    tmp->value = value;
    tmp->known.resize( value.size(), 0xFF );

    auto iter = cst_set.insert( tmp );
    if ( not iter.second )
        delete *iter.first;

    return Expr( const_cast<Cst *>( *iter.first ), 0 );
}
