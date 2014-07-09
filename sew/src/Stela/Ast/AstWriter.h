#ifndef Ast_WRITER_H
#define Ast_WRITER_H

#include "../System/BinStreamWriter.h"
#include "../System/ErrorList.h"
#include <map>
#include <set>
class Ast;

/**
*/
class AstWriter {
public:
    AstWriter( Ast *root );

    ST   size_of_binary_data(); ///< may be called to reserve the size for ptr for copy_to( ptr );
    void copy_binary_data_to( PI8 *ptr );

    // helpers
    void push_nstring ( const String &str );
    void push_delayed_parse( const Ast *l );
    void push_potential_catched_vars_from( const Ast *l );

    // output
    BinStreamWriter data;

protected:
    typedef SI64 OffsetType;
    friend class Ast;

    struct DelayedParse {
        OffsetType  *offset;
        int          old_size;
        const Ast   *l;
    };

    struct IntToReduce {
        enum Type { NOPROP, OFFSET };
        ST   pos; // offset in data (assuming 64 bits before the reduction)
        Type type;
    };

    void parse( Ast *root );
    void int_reduction();
    int  nstring( const String &str );

    // context
    SplittedVec<DelayedParse,32> delayed_parse;
    SplittedVec<IntToReduce,32>  int_to_reduce;
    std::map<std::string,int>    nstrings;
};

#endif // Ast_WRITER_H
