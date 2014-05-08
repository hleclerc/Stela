#include "ErrorList.h"
#include "ReadFile.h"

#include <stdlib.h>

ErrorList::Provenance::Provenance( const char *beg, const char *pos, String provenance, std::string msg ) : provenance( provenance ), msg( msg ) {
    _init( beg, pos );
}

ErrorList::Provenance::Provenance( const char *src, int off, std::string msg ) : provenance( src ), msg( msg ) {
    if ( src and off >= 0 ) {
        ReadFile rf( src );
        _init( rf.data, rf.data + off );
    } else {
        line = -1;
        col  = -1;
    }
}

ErrorList::Provenance::Provenance( String provenance, int line ) : provenance( provenance ), line( line ) {
    col = -1;
}

void ErrorList::Provenance::_init( const char *beg, const char *pos ) {
    if ( not pos ) {
        col  = 0;
        line = 0;
        return;
    }

    const char *b = pos, *e = pos;
    while ( b >= beg and *b != '\n' and *b != '\r' )
        --b;
    ++b;
    while ( *e and *e != '\n' and *e != '\r' )
        ++e;
    if ( b > e )
        b = e;

    complete_line = String( b, e );

    col = pos - b + 1;
    line = 1;
    for ( b = pos; b >= beg; --b )
        line += ( *b == '\n' );
}

void display_line( Stream &os, const char *complete_line, unsigned length_complete_line, int col, bool display_col ) {
    if ( display_col )
        os << "  ";
    if ( length_complete_line < 64 ) {
        os.write(complete_line,length_complete_line);
        if ( display_col ) {
            os << "\n";
            for ( int i = 1;i < 2 + col;++i )
                os << " ";
        }
    } else {
        if ( length_complete_line - col < 64 ) { // only the ending
            os << "...";
            os.write( complete_line + length_complete_line - 64, 64 );
            if ( display_col ) {
                os << "\n";
                for ( unsigned i = 1;i < 2 + 64 + 3 - length_complete_line + col;++i )
                    os << " ";
            }
        } else if ( col < 64 ) { // only the beginning
            os.write( complete_line, 64 );
            os << "...";
            if ( display_col ) {
                os << "\n";
                for ( int i = 1;i < 2 + col;++i )
                    os << " ";
            }
        } else { // middle
            os << "...";
            os.write( complete_line + col - 32, 61 );
            os << "...";
            if ( display_col ) {
                os << "\n";
                for ( int i = 1;i < 2 + 32 + 3;++i )
                    os << " ";
            }
        }
    }
    if ( display_col )
        os << "^";
}

ErrorList::Error::Error() {
}

ErrorList::Error &ErrorList::Error::ac( const char *beg, const char *pos, std::string provenance ) {
    caller_stack.push_back( beg, pos, provenance );
    return *this;
}

ErrorList::Error &ErrorList::Error::ac( const char *src, int off ) {
    caller_stack.push_back( src, off );
    return *this;
}

ErrorList::Error &ErrorList::Error::ap( const char *beg, const char *pos, std::string provenance, std::string msg ) {
    possibilities.push_back( beg, pos, provenance, msg );
    return *this;
}

ErrorList::Error &ErrorList::Error::ap( const char *src, int off, std::string msg ) {
    possibilities.push_back( src, off, msg );
    return *this;
}

void ErrorList::Error::write_to_stream( Stream &os ) const {
    // last item in caller stack
    if ( caller_stack.size() ) {
        if ( display_escape_sequences )
            os << "\033[1m";
        const ErrorList::Provenance &po = caller_stack[ 0 ];
        if ( po.provenance.size() )
            os << po.provenance << ":";
        if ( po.line )
            os << po.line << ":" << po.col << ": ";
        os << "error: " << msg << ( display_col ? "\n" : " in '" );
        if ( po.complete_line.size() )
            display_line( os, po.complete_line.c_str(), po.complete_line.size(), po.col, display_col );
        os << ( display_col ? "\n" : "'\n" );
        if ( display_escape_sequences )
            os << "\033[0m";
    }
    else {
        if ( display_escape_sequences )
            os << "\033[1m";
        os << "error: " << msg << "\n";
        if ( display_escape_sequences )
            os << "\033[0m";
    }

    // caller_stack
    for( int num_prov = 1; num_prov < caller_stack.size(); ++num_prov ) {
        const ErrorList::Provenance &po = caller_stack[ num_prov ];
        if ( po.provenance.size() )
            os << po.provenance << ":";
        if ( po.line ) {
            os << po.line << ":" << po.col;
            //while ( num_prov>0 and error.caller_stack[ num_prov-1 ].line==po.line ) os << "," << error.caller_stack[ --num_prov ].col;
            os << ": ";
        }
        os << "instantiated from: ";
        display_line( os, po.complete_line.c_str(), po.complete_line.size(), po.col, false );
        os << "\n";
    }

    // possibilities
    if ( possibilities.size() ) {
        os << "Possibilities are:" << std::endl;
        for ( int i = 0; i < possibilities.size();++i ) {
            for( int j = 0; ; ++j ) {
                if ( j==i ) {
                    const ErrorList::Provenance & po = possibilities[ i ];
                    if ( po.provenance.size() and po.line )
                        os << "" << po.provenance << ":" << po.line << ":" << po.col << ": ";
                    else
                        os << "(in primitive functions or classes)";
                    if ( po.msg.size() )
                        os << po.msg << ": ";
                    display_line( os, po.complete_line.c_str(), po.complete_line.size(), po.col, false );
                    os << "\n";
                    break;
                }
                if ( possibilities[i].provenance == possibilities[j].provenance and
                     possibilities[i].line == possibilities[j].line and
                     possibilities[i].col  == possibilities[j].col )
                    break;
            }
        }
    }
}

static bool term_supports_color() {
    const char *c_term = getenv("TERM");
    if ( not c_term )
        return false;
    String term = c_term;
    return term != "dumb";
}


ErrorList::ErrorList() {
    display_escape_sequences = term_supports_color();
}

ErrorList::Error &ErrorList::add( String msg, bool warn, bool due_to_not_ended_expr ) {
    Error *e = error_list.push_back();
    e->display_escape_sequences = display_escape_sequences;
    e->due_to_not_ended_expr = due_to_not_ended_expr;
    e->display_col = true;
    e->warn = warn;
    e->msg = msg;

    return *e;
}

void ErrorList::write_to_stream( Stream &os ) const {
    for ( ST i = 0; i < error_list.size(); ++i )
        os << error_list[ i ];
}

int ErrorList::size() const {
    return error_list.size();
}
