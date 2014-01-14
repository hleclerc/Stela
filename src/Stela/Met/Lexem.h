#ifndef LEXEM_H
#define LEXEM_H

#include "../System/TypeConfig.h"

/** */
class Lexem {
public:
    enum {
        VARIABLE        = -1, // must be -1 ( @see num_operator_le )
        NUMBER          = -2,
        STRING          = -3,
        CR              = -4,
        PAREN           = -5,
        CCODE           = -6,
        BLOCK           = -9,
        NONE            = -10,
        APPLY           = -12,
        SELECT          = -13,
        CHANGE_BEHAVIOR = -14,

        NB_BASE_TYPE    = 16
    };

    Lexem();

    template<class Os>
    void write_to_stream( Os &os ) const { os.write( beg, len ); }

    void   to_rat( int &num, int &den ) const; ///< assuming this is a number
    int    to_int   () const; ///< assuming this is a number
    PI64   to_PI64  () const; ///< assuming this is a number
    bool   is_an_int() const;
    bool   eq( const char *p ) const;
    bool   begin_with( const char *p ) const;

    int type; /// >0 => operator. <=0 => @see enum
    int num;  /// Used by type==CR for nb_spaces. If type==CCODE, num in size_cvar

    const char *beg_src; /// provenance data (beginning of the .met data)
    const char *src; /// provenance

    const char *beg; /// beginning in .met
    int         len; /// size in .met

    Lexem *children[ 2 ], *parent, *next, *prev, *sibling, *prev_chro;

    bool preceded_by_a_cr; ///<
    int  nb_preceding_comma_dot;
    int  nb_preceding_cr;
    int  approx_line;
    int  spcr;
};

inline int is_in_main_block( const Lexem *t ) { while( t->prev ) t = t->prev; return not t->parent; }
inline int need_larg( const Lexem *t ) { return t and t->type>=0 and ( t->num & 1 ); }
inline int need_rarg( const Lexem *t ) { return t and t->type>=0 and ( t->num & 2 ); }

void display_graph( const Lexem *t, const char *file_name = "tmp/res.dot", const Lexem *avoid = 0 );
const Lexem *leftmost_child ( const Lexem *t );
const Lexem *rightmost_child( const Lexem *t );


/// a,b,c -> [a b c]
template<class TL>
void get_children_of_type( const Lexem *t, int type, TL &res ) {
    if ( not t )
        return;
    if ( t->type == type ) {
        get_children_of_type( t->children[ 0 ], type, res );
        res.push_back( t->children[ 1 ] );
        return;
    }
    res.push_back( t );
}

// a,b,c -> 3
inline unsigned nb_children_of_type( const Lexem *t, int type ) {
    if ( not t )
        return 0 ;
    return t->type == type ? nb_children_of_type( t->children[ 0 ], type ) + 1 : 1;
}

//int offset_to_doc_of( const Lexem *l, const char *sar_txt );

inline const Lexem *child_if_block(const Lexem *t) { if ( t->type == Lexem::BLOCK ) return t->children[ 0 ]; return t; } ///
inline       Lexem *child_if_block(      Lexem *t) { if ( t->type == Lexem::BLOCK ) return t->children[ 0 ]; return t; } ///

inline const Lexem *child_if_paren(const Lexem *t) { if ( t->type == Lexem::PAREN ) return t->children[ 0 ]; return t; } ///

inline const Lexem *next_if_CR( const Lexem *cc ) {
    if ( cc and cc->type == Lexem::CR )
        return cc->next;
    return cc;
}

#endif // LEXEM_H
