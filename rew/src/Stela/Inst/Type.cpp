#include "../System/Math.h"
#include "Type.h"
#include "Ip.h"

Type::Type( int name ) : name( name ) {
    orig = 0;
    _ali = 1;
    _len = -1;
    _pod = -1;
    _aryth = false;
    _parsed = false;
    _has_a_destructor = false;
}

void Type::write_to_stream( Stream &os ) const {
    os << ip->str_cor.str( name );
    if ( parameters.size() ) {
        os << "[";
        for( int i = 0; i < parameters.size(); ++i ) {
            if ( i )
                os << ",";
            os << parameters[ i ];
        }
        os << "]";
    }
}

void Type::write_C_decl( Stream &out ) const {
    switch( name ) {
    case STRING_Bool_NUM: out << "typedef bool Bool;\n"; break;
    case STRING_SI8_NUM : out << "typedef signed char SI8;\n" ; break;
    case STRING_PI8_NUM : out << "typedef unsigned char PI8;\n" ; break;
    case STRING_SI16_NUM: out << "typedef short int SI16;\n"; break;
    case STRING_PI16_NUM: out << "typedef unsigned short int PI16;\n"; break;
    case STRING_SI32_NUM: out << "typedef int SI32;\n"; break;
    case STRING_PI32_NUM: out << "typedef unsigned int PI32;\n"; break;
    case STRING_SI64_NUM: out << "typedef int64_t SI64;\n"; break;
    case STRING_PI64_NUM: out << "typedef uint64_t PI64;\n"; break;
    default:
        out << "struct " << *this << " {\n";
        out << "    char data[ " << ( size() + 7 ) / 8 << " ];\n";
        out << "};\n";
    }
}

int Type::pod() const {
    if ( _pod < 0 )
        parse();
    return _pod;
}

int Type::size() const {
    if ( _len < 0 )
        parse();
    return _len;
}

int Type::alig() const {
    if ( _ali < 0 )
        parse();
    return _ali;
}

void Type::parse() const {
    if ( _parsed )
        return;
    _parsed = true;

    if ( not orig->block.tok ) {
        ip->disp_error( "Attempting to parse class '" + ip->str_cor.str( name ) + "' which is not defined." );
        return;
    }

    //
    Scope scope( ip->main_scope.ptr(), "_type_" + to_string( *this ) );
    scope.class_scope = this;

    // template parameters
    for( int i = 0; i < parameters.size(); ++i )
        scope.reg_var( orig->arg_names[ i ], parameters[ i ], true );

    // ancestors
    for( int i = 0; i < orig->ancestors.size(); ++i ) {
        TODO; /// append attributes
        //        Var anc_type = scope.parse( orig->ancestors[ i ].sf, orig->ancestors[ i ].tok );
        //        Var anc = scope.apply( anc_type, 0, 0, 0, 0, 0, Scope::APPLY_MODE_PARTIAL_INST, orig->sf, orig->src_off );
        //        if ( not i )
        //            scope.reg_var( STRING_super_NUM, anc, orig->sf, orig->src_off );
        //        static int super_n[] = {
        //            STRING_super_0_NUM, STRING_super_1_NUM, STRING_super_2_NUM, STRING_super_3_NUM,
        //            STRING_super_4_NUM, STRING_super_5_NUM, STRING_super_6_NUM, STRING_super_7_NUM
        //        };
        //        ASSERT( i < 8, "TODO" );
        //        scope.reg_var( super_n[ i ], anc, orig->sf, orig->src_off );

        //        ancestors << ip->type_info( anc.type->expr() );
    }

    scope.parse( orig->block.sf, orig->block.tok, "parsing class" );

    // attr and size
    _len = scope.base_size;
    _ali = scope.base_alig;

    for( int i = 0; i < scope.local_scope.data.size(); ++i ) {
        int name = scope.local_scope.data[ i ].name;
        Var &var = scope.local_scope.data[ i ].var;

        _ali = ppcm( _ali, var.type->_ali );

        if ( var.type->_len >= 0 and _len >= 0 )
            _len = ceil( _len, var.type->_ali );
        else
            _len = -1;

        Attr *attr = _attributes.push_back();
        attr->offset = _len;
        attr->name   = name;
        attr->var    = var;
        attr->sf     = scope.local_scope.data[ i ].sf;
        attr->off    = scope.local_scope.data[ i ].off;

        if ( var.type->_len >= 0 and _len >= 0 )
            _len += var.type->_len;
    }

    for( int i = 0; i < scope.static_scope->data.size(); ++i ) {
        Attr *attr = _attributes.push_back();
        attr->offset = -2;
        attr->name   = scope.static_scope->data[ i ].name;
        attr->var    = scope.static_scope->data[ i ].var;
        attr->sf     = scope.static_scope->data[ i ].sf;
        attr->off    = scope.static_scope->data[ i ].off;
    }

    // has_a_...
    for( int i = 0; i < _attributes.size(); ++i )
        _has_a_destructor |= _attributes[ i ].name == STRING_destroy_NUM;
    for( int i = 0; i < _ancestors.size(); ++i )
        _has_a_destructor |= _ancestors[ i ]->_has_a_destructor;
}

const Type::Attr *Type::find_attr( int name ) const {
    parse();
    for( int i = _attributes.size() - 1; i >= 0; --i )
        if ( _attributes[ i ].name == name )
            return &_attributes[ i ];
    return 0;
}

void Type::find_attr( Vec<const Attr *> &res, int name ) {
    parse();
    for( int i = _attributes.size() - 1; i >= 0; --i )
        if ( _attributes[ i ].name == name )
            res << &_attributes[ i ];
}

Var Type::make_attr( Var self, const Attr *attr ) const {
    if ( attr->offset < 0 )
        return attr->var;
    return ( self.ptr() + Var( attr->offset ) ).at( attr->var.type );
}

