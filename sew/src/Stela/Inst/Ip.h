#ifndef IP_H
#define IP_H

#include "../System/ErrorList.h"
#include "../System/NstrCor.h"
#include "Sourcefile.h"
struct Type;
struct Expr;

/**
*/
class Ip {
public:
    struct CS { SourceFile *sf; int off; const char *reason; };

    Ip();
    ~Ip();

    Expr ret_error( String msg, bool warn = false, const char *file = 0, int line = 0 );
    void disp_error( String msg, bool warn = false, const char *file = 0, int line = 0 );
    ErrorList::Error &error_msg( String msg, bool warn = false, const char *file = 0, int line = 0 );

    Expr error_var();


    #define DECL_BT( T ) Type *type_##T;
    #include "DeclBaseClass.h"
    #undef DECL_BT

    std::map<String,SourceFile> sourcefiles;

    int         off;      ///< current offset in sourcefile
    SourceFile *sf;       ///< current sourcefile
    Vec<CS>     sf_stack;

    ErrorList   error_list;
    NstrCor     str_cor;
};

extern Ip *ip;

#endif // IP_H
