#ifndef STELA_AST_Callable_H
#define STELA_AST_Callable_H

#include "../System/SplittedVec.h"
#include "../System/AutoPtr.h"
#include "../System/Vec.h"
#include "Ast.h"

/**
*/
class Ast_Callable : public Ast {
public:
    typedef SplittedVec<Past,4> DVT;

    Ast_Callable( int off );

    virtual void get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const;
    virtual void prep_get_potentially_needed_ext_vars( std::set<String> &avail ) const;
    virtual void write_to_stream( Stream &os, int nsp = 0 ) const;
    virtual void write_callable_type( Stream &os ) const = 0;
    double       default_pertinence() const;

    int min_nb_args() const;
    int max_nb_args() const;

    virtual void _get_potentially_needed_ext_vars( std::set<String> &res, std::set<String> &avail ) const = 0;
    virtual Expr _parse_in( ParsingContext &context ) const;
    virtual void _get_info( IrWriter *aw ) const;
    virtual int  _spec_flags() const = 0;


    String            name;
    bool              self_as_arg;
    bool              abstract;
    bool              varargs;
    bool              stat;
    Past              pertinence;
    Past              condition;
    int               def_pert_num;
    int               def_pert_den;
    Vec<String>       arguments;
    Vec<Vec<String> > arg_constraints; ///< same size than arguments
    DVT               default_values;
    Past              block;

    String            sourcefile;
};

#endif // STELA_AST_Callable_H

