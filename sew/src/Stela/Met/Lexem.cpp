#include "../System/UsualStrings.h"
#include "../System/Assert.h"
#include "TypeChar.h"
#include "Lexem.h"

#include <stdlib.h>
#include <string.h>
#include <fstream>

Lexem::Lexem() {
    children[ 0 ]    = 0;
    children[ 1 ]    = 0;
    parent           = 0;
    next             = 0;
    prev             = 0;
    sibling          = 0;
    prev_chro        = 0;
    num_in_scope     = -1;
    num_scope        = -1;
    preceded_by_a_cr = false;

    nb_preceding_comma_dot = 0;
    nb_preceding_cr        = 0;
}

void Lexem::to_rat( int &num, int &den ) const {
    num = 0;
    den = 1;
    for( int i = 0; i < len; ++i ) {
        if ( beg[ i ] == '.' ) {
            for( int j = i + 1; j < len; ++j ) {
                num = num * 10 + ( beg[ j ] - '0' );
                den *= 10;
            }
            break;
        }
        num = num * 10 + ( beg[ i ] - '0' );
    }
}

int Lexem::to_int() const {
    int res = 0;
    for( int i = 0; i < len and beg[ i ] >= '0' and beg[ i ] <= '9'; ++i )
        res = res * 10 + ( beg[ i ] - '0' );
    return res;
}

PI64 Lexem::to_PI64() const {
    PI64 res = 0;
    for( int i = 0; i < len and beg[ i ] >= '0' and beg[ i ] <= '9'; ++i )
        res = res * 10 + ( beg[ i ] - '0' );
    return res;
}

bool Lexem::is_an_int() const {
    if ( type != NUMBER )
        return false;
    for( int i = 0; i < len; ++i )
        if ( type_char( beg[ i ] ) != TYPE_CHAR_number )
            return false;
    return true;
}

bool Lexem::eq( const char *p ) const {
    return int( strlen( p ) ) == len and strncmp( p, beg, len ) == 0;
}

bool Lexem::begin_with( const char *p ) const {
    return int( strlen( p ) ) <= len and strncmp( p, beg, strlen( p ) ) == 0;
}

bool Lexem::same_str( const char *p, int l ) const {
    return len == l and strncmp( beg, p, l ) == 0;
}

const Lexem *leftmost_child( const Lexem *t ) {
    if ( t->type == Lexem::APPLY or t->type == Lexem::SELECT or t->type == Lexem::CHANGE_BEHAVIOR )
        return leftmost_child( t->children[  0] ); // a(15)
    if ( t->type > 0 )
        if ( ( t->num & 1 )/*need_left_arg*/ or ( ( t->num & 4 )/*MayNeedlarg*/ and t->children[0] ) )
            return leftmost_child( t->children[ 0 ] ); // a', a-b
    return t;
}

const Lexem *rightmost_child( const Lexem *t ) {
    while ( t->next )
       t = t->next;

    if ( t->type==Lexem::PAREN or t->type==Lexem::APPLY or t->type==Lexem::SELECT or t->type==Lexem::CHANGE_BEHAVIOR )
        return t;

    if ( t->children[ 1 ] )
        return rightmost_child( t->children[ 1 ] );
    if ( t->children[ 0 ] and not ( t->type > 0 and ( t->num & 1/*need_left_arg*/ ) ) ) // operators with 1 child are in left of there operand
        return rightmost_child( t->children[ 0 ] );
    return t;
}


void display_graph_rec( std::ostream &os, const Lexem *t, unsigned level, unsigned &max_level, const Lexem *avoid ) {
    if ( t == avoid )
        return;

    unsigned j;
    max_level = std::max( max_level, level );
    const Lexem *old_t = t;
    // nodes
    for( ; t ; t = t->next ) {
        os << "{ rank = same; " << level << " node" << t << " [label=\"";
        //if ( t->preceded_by_a_cr ) os << "[CR]";
        //if ( t->nb_preceding_comma_dot )
        //    os << "[;=>" << t->nb_preceding_comma_dot << ']';
        //if ( t->nb_preceding_cr )
        //    os << "[n=>" << t->nb_preceding_cr << ']';
        if ( t->num_scope >= 0 ) {
            os << '[' << t->num_scope;
            if ( t->num_in_scope >= 0 )
                os << ',' << t->num_in_scope;
            os << ']';
        }
        for( int i = 0; i < t->len; ++i ) {
            if ( t->beg[ i ]=='\n' )
                os << "\\n";
            else if ( t->beg[ i ]=='"' )
                os << "\\\"";
            else
                os << t->beg[ i ];
        }

        switch ( t->type ) {
        case Lexem::CR             : os << "CR" << t->num; break;
        case Lexem::BLOCK          : os << "BLOCK"; break;
        case Lexem::NONE           : os << "NONE"; break;
        case Lexem::APPLY          : os << "APPLY"; break;
        case Lexem::SELECT         : os << "SELECT"; break;
        case Lexem::CHANGE_BEHAVIOR: os << "CHANGE_BEHAVIOR"; break;
        // case STRING___if___NUM     : os << "If"; break;
        case STRING_not_boolean_NUM: os << "!"; break;
        }

        os << "\"] }\n";

        // children
        for( j = 0; j < 2; ++j ) {
            if ( t->children[ j ] and t->children[ j ] != avoid ) {
                display_graph_rec( os, t->children[ j ], level + 1, max_level, avoid );
                os << "  node" << t << " -> node" << t->children[ j ] << ";\n";
            }
        }
        // ascending edge
        if ( t->parent and t->parent != avoid )
            os << "  node" << t << " -> node" << t->parent << " [color=red];\n";
    }
    // edges
    t = old_t;
    for( ; t ; t = t->next ) {
        if ( t->next and t->next != avoid )
            os << "  node" << t << " -> node" << t->next << " [color=green];\n";
        if ( t->prev and t->prev != avoid ) //
            os << "  node" << t << " -> node" << t->prev << " [color=yellow];\n";
    }
}

void display_graph( const Lexem *t, const char *file_name, const Lexem *avoid ) {
    std::ofstream f(file_name);
    f << "digraph popoterie {";
    unsigned max_level = 1;
    display_graph_rec( f, t, 1, max_level, avoid );

    for( unsigned i = 1; i <= max_level; ++i )
        f << "  " << i << " [ shape=plaintext ];\n  ";
    for( unsigned i = 1; i <= max_level; ++i )
        f << i << ( i<max_level ? " -> " : ";" );

    f << "}";
    f.close();

    int res = system( ("dot -Tps "+std::string(file_name)+" > "+std::string(file_name)+".eps && gv "+std::string(file_name)+".eps").c_str() );
    if ( res )
        res = 0;
}

