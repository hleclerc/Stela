#ifndef CONVTYPE_H
#define CONVTYPE_H

#include "../System/StreamSep.h"
#include "../System/Stream.h"
#include "../Ast/Ast.h"
class Ast_Class;

/**
*/
class ConvType {
public:
    struct Attribute {
        String    name;
        ConvType *type; ///< offset in bits (NULL if static)
        Past      def;  ///< default value (for the constructor)
    };

    ConvType( Ast_Class *ast_class );

    void write_to_stream( Stream &os ) const;
    void parse();

    void write_cpp_decl( StreamSepMaker<Stream> &on );
    String cpp_name() const;

    Ast_Class     *ast_class;
    Vec<Past>      args;
    Vec<Attribute> attributes;
    Vec<Attribute> staticvars;

    Past           base_size;
    Past           base_alig;
};

#endif // CONVTYPE_H
