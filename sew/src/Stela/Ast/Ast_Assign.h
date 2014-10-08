#ifndef STELA_AST_Assign_H
#define STELA_AST_Assign_H

#include "../System/AutoPtr.h"
#include "Ast.h"

/**
*/
class Ast_Assign : public Ast {
public:
    Ast_Assign( int off );

    virtual void get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const;
    virtual void prep_get_potentially_needed_ext_vars( std::set<String> &avail ) const;
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;

protected:
    friend class AstMaker;
    friend class ConvType;

    virtual void _get_info( IrWriter *aw ) const;
    virtual PI8  _tok_number() const;
    virtual Past _parse_in( ConvScope &scope );

    String name;
    Past   val;
    bool   stat;
    bool   cons;
    bool   type; ///< true if "... ~= ..."
    bool   ref;
};

#endif // STELA_AST_Assign_H

