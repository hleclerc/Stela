#ifndef AST_H
#define AST_H

#include "../System/Stream.h"
#include "../System/Vec.h"
#include "../System/Ptr.h"
#include <set>
#include <map>
class ParsingContext;
class ErrorList;
class IrWriter;
class Lexem;
class Expr;

/***/
class Ast : public ObjectWithCptUse {
public:
    typedef Ptr<Ast> Past;

    Ast( const char *src = 0, int off = -1 );
    virtual ~Ast();

    virtual void get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const;
    virtual void prep_get_potentially_needed_ext_vars( std::set<String> &avail ) const; ///< for class blocks, to get attributes before parsing defs
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const = 0;
    void write_to( IrWriter *aw ) const;

    virtual bool may_be_surdefined() const;

    Expr parse_in( ParsingContext &context ) const;

    virtual Expr _parse_in( ParsingContext &context ) const = 0;
    virtual void _get_info( IrWriter *aw ) const;
    virtual PI8  _tok_number() const;

    static std::set<String> sf_set; ///< set of sourcefile

    int           _off; ///< offset in sourcefile
    const String *_src; ///> sourcefile
};

typedef Ptr<Ast> Past;

/// sibling = true to read l, l->next, l->next->next, ...
Past make_ast( ErrorList &e, const Lexem *l, bool sibling );

#endif // AST_H
