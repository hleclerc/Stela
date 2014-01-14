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
