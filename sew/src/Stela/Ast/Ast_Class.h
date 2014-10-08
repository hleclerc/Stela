#ifndef STELA_AST_Class_H
#define STELA_AST_Class_H

#include "Ast_Callable.h"

/**
*/
class Ast_Class : public Ast_Callable {
public:
    Ast_Class( int off );

    virtual void write_callable_type( Stream &os ) const { os << "class"; }
    virtual ConvType *make_type_for( Vec<Past> &args );
    virtual ConvType *make_type();

protected:
    friend class AstMaker;

    virtual void _get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const;
    virtual void _get_info( IrWriter *aw ) const;
    virtual PI8  _tok_number() const;
    virtual int  _spec_flags() const;

    SplittedVec<Past,2> inheritance;
};

#endif // STELA_AST_Class_H

