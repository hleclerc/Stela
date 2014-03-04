#include "../Inst/Slice.h"
#include "Interpreter.h"
#include "TypeInfo.h"
#include "RefSlice.h"
#include "Scope.h"

TypeInfo::TypeInfo( ClassInfo *orig ) : orig( orig ), parsed( false ) {
}

void TypeInfo::parse_if_necessary() {
    if ( parsed )
        return;
    parsed = true;

    //
    Scope scope( ip->main_scope, 0 );
    scope.class_scope = this;

    // template parameters
    for( int i = 0; i < parameters.size(); ++i )
        scope.reg_var( orig->arg_names[ i ], parameters[ i ], orig->sf, orig->src_off, true );

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

    scope.parse( orig->block.sf, orig->block.tok );

    // attr and size
    static_size_in_bits = scope.base_size;
    static_alig_in_bits = scope.base_alig;

    for( int i = 0; i < scope.named_vars.lst.size(); ++i ) {
        int name = scope.named_vars.lst[ i ].name;
        Var &var = scope.named_vars.lst[ i ].var;
        TypeInfo *var_type = ip->type_info( var.type->expr() );

        static_alig_in_bits = ppcm( static_alig_in_bits, var_type->static_alig_in_bits );

        if ( var_type->static_size_in_bits >= 0 and static_size_in_bits >= 0 )
            static_size_in_bits = ceil( static_size_in_bits, var_type->static_alig_in_bits );
        else
            static_size_in_bits = -1;

        Attr *attr = attributes.push_back();
        attr->offset = static_size_in_bits;
        attr->name   = name;
        attr->var    = var;

        if ( var_type->static_size_in_bits >= 0 and static_size_in_bits >= 0 )
            static_size_in_bits += var_type->static_size_in_bits;
    }

    for( int i = 0; i < scope.static_named_vars->lst.size(); ++i ) {
        Attr *attr = attributes.push_back();
        attr->offset = -2;
        attr->name   = scope.static_named_vars->lst[ i ].name;
        attr->var    = scope.static_named_vars->lst[ i ].var;
    }

    // has_a_...
    for( int i = 0; i < attributes.size(); ++i )
        has_a_destructor |= attributes[ i ].name == STRING_destroy_NUM;
    for( int i = 0; i < ancestors.size(); ++i )
        has_a_destructor |= ancestors[ i ]->has_a_destructor;
}

const TypeInfo::Attr *TypeInfo::find_attr( int name ) {
    for( int i = attributes.size() - 1; i >= 0; --i )
        if ( attributes[ i ].name == name )
            return &attributes[ i ];
    return 0;
}

void TypeInfo::find_attr( Vec<const Attr *> &res, int name ) {
    for( int i = attributes.size() - 1; i >= 0; --i )
        if ( attributes[ i ].name == name )
            res << &attributes[ i ];
}

Var TypeInfo::make_attr( const Var &self, const Attr *attr ) {
    if ( attr->offset == -2 )
        return attr->var;
    if ( attr->offset == -1 ) ///< dynamic offset
        TODO;
    TypeInfo *ti = ip->type_info( attr->var.type_expr() );
    if ( ti->static_size_in_bits < 0 )
        TODO; ///< dynamic attr size

    Var res;
    res.flags = self.flags;
    res.type  = attr->var.type;
    res.data  = new PRef( new RefSlice( self, attr->offset, attr->offset + ti->static_size_in_bits ) );
    return res;
}
