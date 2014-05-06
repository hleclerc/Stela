#include "../Inst/Expr.h"
#include "Type.h"
#include "Ref.h"

Ref::Ref( Type *type, RefItem *ref_item ) : type( type ), ref_item( ref_item ), flags( 0 ) {
}

Ref::~Ref() {
    delete ref_item;
}

void Ref::write_to_stream( Stream &os ) const {
    if ( type )
        os << *type << "{";
    if ( ref_item )
        os << *ref_item;
    if ( type )
        os << "}";
}

void Ref::reassign( const Ref &src ) {
    ref_item->reassign( *this, src.ref_item );
}

Expr Ref::expr() const {
    return ref_item->expr();
}

