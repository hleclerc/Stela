#ifndef IP_H
#define IP_H

#include "../System/SplittedVec.h"
#include "../System/ErrorList.h"
#include "../System/NstrCor.h"
#include "Type.h"

/**
*/
class Ip {
public:
    typedef SplittedVec<Vec<Ptr<Inst> >,16> OldCondSet;

    Ip();

    // errors
    void disp_error( String msg, bool warn = false, const char *file = 0, int line = -1 );
    ErrorList::Error &error_msg( String msg, bool warn = false, const char *file = 0, int line = -1 );

    void set_cond( const Var &cond );
    void set_cond( Ptr<Inst> cond );
    void pop_cond();

    // base type
    Type  type_SI32;
    Type  type_SI64;
    Type  type_Void;
    Type  type_Bool;
    Type  type_RawPtr;
    Type *type_ST;

    // context
    Vec<Ptr<Inst> > cond_stack;
    Var             sys_state;

    //
    ErrorList error_list;
    NstrCor   str_cor;
};

#define IP_ERROR( MSG ) ip->disp_error( MSG, false, __FILE__, __LINE__ );


extern Ip *ip;

#endif // IP_H
