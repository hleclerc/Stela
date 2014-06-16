#include "../Ir/CallableFlags.h"
#include "../System/Math.h"
#include "Class.h"
#include "Slice.h"
#include "Type.h"
#include "Ip.h"
#include "Op.h"

Type *Type::Attr::get_type() {
    if ( type )
        return ip->type_from_type_var( val );
    return val->ptype();
}

Type *Type::Attr::get_ptr_type() {
    if ( type ) {
        Vec<Expr> v( val );
        return ip->class_Ptr->type_for( v );
    }
    return val->type();
}


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

void Type::write_to_stream( Stream &os, void *data, int len ) {
    int sb = ( len + 7 ) / 8;
    const char *c = "0123456789ABCDEF";
    for( int i = 0; i < std::min( sb, 4 ); ++i )
        os << c[ reinterpret_cast<PI8 *>( data )[ i ] >> 4 ]
           << c[ reinterpret_cast<PI8 *>( data )[ i ] & 0xF ];
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

    _len = 0;
    _ali = 1;

    Scope ns( &ip->main_scope, 0, "parsing type" );
    ns.local_vars.append( parameters );
    ns.callable = orig;
    for( Class::Attribute &a : orig->attributes ) {
        Expr val = ns.parse( a.code.sf, a.code.tok, "arg init" );
        if ( a.type & CALLABLE_ATTR_BASE_ALIG ) {
            int v = 0;
            if ( not val->get( ns.cond )->get_val( ip->type_SI32, &v ) )
                ip->disp_error( "expecting a number known at runtime" );
            _ali = ppcm( _ali, val->type()->alig() );
            continue;
        }
        if ( a.type & CALLABLE_ATTR_BASE_SIZE ) {
            int v = 0;
            if ( not val->get( ns.cond )->get_val( ip->type_SI32, &v ) )
                ip->disp_error( "expecting a number known at runtime" );
            _len += v;
            continue;
        }
        if ( a.type & CALLABLE_ATTR_TYPE ) // ~=
            val = ns.apply( val, 0, 0, 0, 0, 0, Scope::APPLY_MODE_PARTIAL_INST );
        ns.local_vars << val;

        _len = ceil( _len, _ali );

        bool stat = a.type & CALLABLE_ATTR_STATIC;
        attributes << Attr{ stat ? -1 : _len, val, a.type == CALLABLE_ATTR_TYPE };

        _ali = ppcm( _ali, val->ptype()->alig() );
        _len += val->ptype()->size();
    }
}
