#include "../Ast/Ast_Assign.h"
#include "../Ast/Ast_Symbol.h"
#include "../Ast/Ast_Class.h"
#include "../System/dcast.h"
#include "ConvContext.h"
#include "ConvType.h"

ConvType::ConvType( Ast_Class *ast_class ) : ast_class( ast_class ) {
}

void ConvType::write_to_stream( Stream &os ) const {
    os << ast_class->name << "[]\n";
    for( const Attribute &a : attributes )
        if ( a.type )
            os << "    " << a.name << " ~= " << *a.type << "\n";
}

void ConvType::parse() {
    // read in scope
    ConvScope scope( &cc->main_scope );
    cc->file_stack << ast_class->sourcefile;
    ast_class->block->parse_in( scope );

    base_size = scope.base_size;
    base_alig = scope.base_alig;
    for( ConvScope::NamedVar e : scope.variables ) {
        if ( e.stat )
            continue;
        if ( Ast_Assign *aa = dcast( e.expr.data ) ) {
            if ( aa->type ) {
                Past val = aa->val->parse_in( scope );
                ConvType *t = val->make_type();
                attributes << Attribute{ e.name, t, Past() };
            } else {
                TODO;
            }
            // aa->val
        } else
            TODO;
        PRINT( e.name );
        PRINT( e.expr );
    }

    cc->file_stack.pop_back();
}

String ConvType::cpp_name() const {
    if ( args.size() )
        TODO;
    return "Model_" + ast_class->name;
}

void ConvType::write_cpp_decl( StreamSepMaker<Stream> &on ) {
    on << "struct " << cpp_name() << " : public Model {";
    on.nsp += 4;

    // constructors
    on << cpp_name() << "( char  *__ptr, Model *__par, char __bof ) : __ptr( __ptr ), __par( __par ), __bof( __bof ) {";
    on << "}";

    // attributes
    for( const ConvType::Attribute &attr : attributes ) {
        if ( ConvType *t = attr.type ) {
            on << t->cpp_name() << " " << attr.name << "() {";
            on << "    return "<< t->cpp_name() << "( __ptr, __par, __bof );";
            on << "}";
        }
    }

    // size and alig
    Past s = base_size, a = base_alig, p = new Ast_Symbol( "__ptr" ), b = new Ast_Symbol( "__bof" );

    on << "int __size() const {";
    on << "    return " << s << ";";
    on << "}";
    on << "int __alig() const {";
    on << "    return " << a << ";";
    on << "}";

    on << "char  *__ptr;";
    on << "Model *__par;";
    on << "char   __bof;";

    on.nsp -= 4;
    on << "};";
}


