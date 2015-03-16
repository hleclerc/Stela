#include "../Ir/CallableFlags.h"
#include "../System/Math.h"
#include "Class.h"
#include "Slice.h"
#include "Type.h"
#include "Ip.h"
#include "Op.h"

Type::Type( Class *orig ) : orig( orig ) {
    _len    = -1;
    _ali    = -1;
    _pod    = -1;
    _parsed = false;

    aryth   = false;
}

void Type::write_to_stream( Stream &os ) {
    os << *orig;
    if ( parameters.size() ) {
        os << "[" << parameters[ 0 ]->get();
        for( int i = 1; i < parameters.size(); ++i )
            os << "," << parameters[ i ]->get();
        os << "]";
    }
}

void Type::write_to_stream( Stream &os, const void *data, int len ) {
    if ( len == 0 )
        os << "void";
    int sb = ( len + 7 ) / 8;
    const char *c = "0123456789ABCDEF";
    for( int i = 0; i < std::min( sb, 4 ); ++i )
        os << c[ reinterpret_cast<const PI8 *>( data )[ i ] >> 4 ]
           << c[ reinterpret_cast<const PI8 *>( data )[ i ] & 0xF ];
    if ( sb > 4 )
        os << "...";
}

int Type::size() {
    if ( _len < 0 )
        parse();
    return _len;
}

int Type::alig() {
    if ( _ali < 0 )
        parse();
    return _ali;
}

int Type::pod() {
    return _pod;
}

int Type::sb() {
    return ( size() + 7 ) / 8;
}

void Type::parse() {
    if ( _parsed )
        return;
    _parsed = true;

    // parse block
    Scope ns( &ip->main_scope, 0, "parsing type " + to_string( this ) );
    ns.class_scope = this;
    for( int i = 0; i < parameters.size(); ++i )
        ns.reg_var( orig->arg_names[ i ], parameters[ i ], true );
    ns.parse( orig->block.sf, orig->block.tok, "type parsing" );

    _len = ns.base_size;
    _ali = ns.base_alig;

    for( NamedVarList::NamedVar &nv : ns.local_vars.data ) {
        if ( nv.expr->flags & Inst::SURDEF ) {
            attributes << Attr{ -1, nv.expr, nv.name };
        } else {
            _ali = ppcm( _ali, nv.expr->ptype()->alig() );
            _len = ceil( _len, _ali );

            attributes << Attr{ _len, nv.expr, nv.name };

            _len += nv.expr->ptype()->size();
        }
    }

    for( NamedVarList::NamedVar &nv : ns.static_vars->data )
        attributes << Attr{ -1, nv.expr, nv.name };
}
