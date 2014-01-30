#include "Interpreter.h"
#include "TypeInfo.h"
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
        Var anc_type = scope.parse( orig->ancestors[ i ].sf, orig->ancestors[ i ].tok );
        Var anc = scope.apply( anc_type, 0, 0, 0, 0, 0, Scope::APPLY_MODE_PARTIAL_INST, orig->sf, orig->src_off );
        if ( not i )
            scope.reg_var( STRING_super_NUM, anc, orig->sf, orig->src_off );
        static int super_n[] = {
            STRING_super_0_NUM, STRING_super_1_NUM, STRING_super_2_NUM, STRING_super_3_NUM,
            STRING_super_4_NUM, STRING_super_5_NUM, STRING_super_6_NUM, STRING_super_7_NUM
        };
        ASSERT( i < 8, "TODO" );
        scope.reg_var( super_n[ i ], anc, orig->sf, orig->src_off );

        ancestors << ip->type_info( anc.type->expr() );
    }

    scope.parse( orig->block.sf, orig->block.tok );
    stat_attrs = scope.static_named_vars;

    // has_a_...
    if ( stat_attrs )
        for( int i = 0; i < stat_attrs->lst.size(); ++i )
            has_a_destructor |= stat_attrs->lst[ i ].name == STRING_destroy_NUM;
    for( int i = 0; i < ancestors.size(); ++i )
        has_a_destructor |= ancestors[ i ]->has_a_destructor;


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

        Attr *attr = dyna_attrs.push_back();
        attr->offset = static_size_in_bits;
        attr->name   = name;
        attr->var    = var;

        if ( var_type->static_size_in_bits >= 0 and static_size_in_bits >= 0 )
            static_size_in_bits += var_type->static_size_in_bits;
    }
}
