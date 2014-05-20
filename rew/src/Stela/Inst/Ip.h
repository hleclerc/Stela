#ifndef IP_H
#define IP_H

#include "../System/ErrorList.h"
#include "../System/NstrCor.h"
#include "Type.h"
#include "Var.h"
#include <map>

/**
*/
class Ip {
public:
    typedef SplittedVec<Vec<Expr>,16> OldCondSet;

    Ip();

    Var ret_error( String msg, bool warn = false, const char *file = 0, int line = -1 );
    void disp_error( String msg, bool warn = false, const char *file = 0, int line = -1 );
    ErrorList::Error &error_msg( String msg, bool warn = false, const char *file = 0, int line = -1 );

    void set_cond( Expr cond );
    void set_cond( Var cond );
    Expr cur_cond();
    void pop_cond();

    Var error_var();

    Type *artificial_type_for_size( int size );

    // base type
    Type  type_SI32;
    Type  type_SI64;
    Type  type_Void;
    Type  type_Bool;
    Type  type_Error;
    Type  type_RawPtr;
    Type *type_ST;

    std::map<int,Type> art_types;

    // std variables
    Expr cst_false;
    Expr cst_true;

    // context
    Vec<Expr> cond_stack;
    Var       sys_state;

    //
    ErrorList error_list;
    NstrCor   str_cor;
};

extern Ip *ip;

#endif // IP_H
