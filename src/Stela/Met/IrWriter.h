#ifndef Ast_WRITER_H
#define Ast_WRITER_H

#include "../System/BinStreamWriter.h"
#include "../System/ErrorList.h"
#include <map>
#include <set>
class Ast;

/**
*/
class IrWriter {
public:
    IrWriter( ErrorList &error_list );
    void parse( Ast *root );

    ST   size_of_binary_data(); ///< may be called to reserve the size for ptr for copy_to( ptr );
    void copy_binary_data_to( PI8 *ptr );

protected:
    typedef SI64 OffsetType;

    struct DelayedParse {
        OffsetType  *offset;
        int          old_size;
        Ast         *l;
    };

    struct IntToReduce {
        enum Type { NOPROP, OFFSET };
        ST   pos; // offset in data (assuming 64 bits before the reduction)
        Type type;
    };

    void parse_lexem_and_siblings( const Lexem *l );
    void parse_lexem             ( const Lexem *l ); ///< individual lexem

    void parse_apply             ( const Lexem *l, PI8 tok );
    void parse_number            ( const Lexem *l );
    void parse_string            ( const Lexem *l );
    void parse_paren             ( const Lexem *l );
    void parse_variable          ( const Lexem *l );
    void parse_assign            ( const Lexem *l, bool assign_type = false );
    void parse_reassign          ( const Lexem *l );
    void parse_def               ( const Lexem *l );
    void parse_class             ( const Lexem *l );
    void parse_if                ( const Lexem *l );
    void parse_while             ( const Lexem *l );
    void parse_return            ( const Lexem *l );
    void parse_get_attr          ( const Lexem *l, PI8 tok );
    void parse_static            ( const Lexem *l );
    void parse_const             ( const Lexem *l );
    void parse_call_op           ( const Lexem *l );
    void parse_break             ( const Lexem *l );
    void parse_continue          ( const Lexem *l );
    void parse_for               ( const Lexem *l );
    void parse_import            ( const Lexem *l );
    void parse_lambda            ( const Lexem *l );
    void parse_and               ( const Lexem *l );
    void parse_or                ( const Lexem *l );

    void push_delayed_parse      ( const Lexem *l, bool want_siblings = true );
    void int_reduction           ();

    // helpers
    void add_error               ( String msg, const Lexem *l );
    void parse_callable          ( const Lexem *l, PI8 token_type );
    void push_offset             ( const Lexem *l );
    int  nstring                 ( const char *beg, const char *end );
    int  nstring                 ( const char *beg );
    int  nstring                 ( const Lexem *l );
    void push_nstring            ( const Lexem *l );
    void push_nstring            ( int nstring_num );
    void output_list             ( const Lexem *l, int nb_dim, bool has_cd );

    void find_needed_var         ( Vec<CatchedVar> &cl, const Lexem *v );
    void get_needed_var_rec      ( std::map<String,CatchedVarWithNum> &vars, const Lexem *b, int onp );
    void out_catched_vars        ( std::map<String,CatchedVarWithNum> &catched_vars, int num_scope );

    BinStreamWriter data;
    ErrorList &error_list;

    // context
    SplittedVec<DelayedParse,32> delayed_parse;
    SplittedVec<IntToReduce,32> int_to_reduce;
    std::map<std::string,int> nstrings;
    std::map<const Lexem *,std::map<String,CatchedVarWithNum> > catched; ///< catched[ callable ] -> catched vars (name -> num in catched var...)
    bool static_inst, const_inst;
};

#endif // Ast_WRITER_H
