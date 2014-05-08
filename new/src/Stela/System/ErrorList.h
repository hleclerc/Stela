#ifndef ERRORLIST_H
#define ERRORLIST_H

#include "SplittedVec.h"
#include "Stream.h"
#include "Vec.h"

/**
*/
class ErrorList {
public:
    struct Provenance {
        Provenance( const char *beg, const char *pos, std::string provenance, std::string msg = "" );
        Provenance( const char *src, int off, std::string msg = "" );
        Provenance( String provenance, int line );

        void _init( const char *beg, const char *pos );

        String      provenance; /// name of file
        String      complete_line;
        int         line;
        int         col;
        String      msg;
    };

    struct Error {
        Error();
        void write_to_stream( Stream &os ) const;

        Error &ac( const char *beg, const char *pos, std::string provenance ); ///< add a caller
        Error &ac( const char *src, int off );

        Error &ap( const char *beg, const char *pos, std::string provenance, std::string msg = "" ); ///< add a possibility
        Error &ap( const char *src, int off, std::string msg = "" );

        bool due_to_not_ended_expr, display_col, display_escape_sequences, warn;
        Vec<Provenance> caller_stack;  /// "copy" of caller stack
        Vec<Provenance> possibilities; /// if ambiguous overload, list of possible functions
        std::string msg;
    };

    ErrorList();

    Error &add( String msg, bool warn = false, bool due_to_not_ended_expr = false );
    operator bool() const { return error_list.size(); }
    void write_to_stream( Stream &os ) const;
    int size() const;

protected:
    SplittedVec<Error,4> error_list;
    bool display_escape_sequences;
};


#endif // ERRORLIST_H
