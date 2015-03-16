#ifndef LEXER_H
#define LEXER_H

#include "../System/UsualStrings.h"
#include "../System/ErrorList.h"
#include "Lexem.h"

/**
*/
class Lexer {
public:
    Lexer( ErrorList &error_list );
    void parse( const char *beg, const char *src );
    void display(); ///< graphviz display
    Lexem *root();

    void write_error( const std::string &msg, const char *beg, const char *pos, const std::string &src, bool due_to_not_ended = false );
protected:
    template<bool use_stop_condition> void read_s( const char *limit = 0 );
    Lexem *new_tok( int type, const char *beg, unsigned len, int num );
    Lexem *app_tok( int type, const char *beg, unsigned len, int num );
    void read_dollar( const char *old_str, unsigned nb_spaces = 0 );
    bool assemble_2_args( Lexem *o, int need_left, int need_right );
    bool assemble_right_arg( Lexem *o );
    bool assemble_left_arg( Lexem *o );
    void make_calls_without_par_rec( Lexem *l );

    void make_flat_token_list();
    void not_in_replacement();
    void make_sibling_lists();
    void set_next_prev_fields();
    void parenthesis_hierarchisation( const Lexem *ft ); ///< mark data into (), [] and {} as children
    void make_unit_hierarchy();
    void remove_cr();
    void assemble___calc_name__();
    void assemble___string_assembly__();
    void assemble_par___getattr__( const Lexem *ft );
    void assemble_op( int mi, int ma );
    void add_child_to_info( int stn );
    void make_calls_without_par();
    void remove_comma_dots();
    void update_else_order();
    void remove_while_conds();

    const char *s; ///<
    const char *beg; ///<
    const char *src; ///< provenance

    Lexem *first_of_type[ NB_OPERATORS + Lexem::NB_BASE_TYPE ]; ///
    Lexem *last_of_type [ NB_OPERATORS + Lexem::NB_BASE_TYPE ]; ///
    Lexem first_tok, *end;
    int approx_line;
    int spcr;

    SplittedVec<Lexem,64> lexems;
    ErrorList &error_list;
};

#endif // LEXER_H
