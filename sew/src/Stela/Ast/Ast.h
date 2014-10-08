#ifndef AST_H
#define AST_H

#include "../System/Stream.h"
#include "../System/Vec.h"
#include "../System/Ptr.h"
#include <set>
class ErrorList;
class ConvScope;
class IrWriter;
class ConvType;
class Lexem;

/***/
class Ast : public ObjectWithCptUse {
public:
    typedef Ptr<Ast> Past;

    Ast( int off = -1 );
    virtual ~Ast();

    virtual void get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const;
    virtual void prep_get_potentially_needed_ext_vars( std::set<String> &avail ) const;
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const = 0;
    void write_to( IrWriter *aw ) const;

    Past parse_in( ConvScope &scope );
    virtual bool may_be_surdefined() const;
    virtual ConvType *make_type();

protected:
    friend class ConvExpr;

    virtual void _get_info( IrWriter *aw ) const;
    virtual PI8  _tok_number() const;
    virtual Past _parse_in( ConvScope &scope );

    int _off; ///< offset in sourcefile
};

typedef Ptr<Ast> Past;

/// sibling = true to read l, l->next, l->next->next, ...
Past make_ast( ErrorList &e, const Lexem *l, bool sibling );

#endif // AST_H
