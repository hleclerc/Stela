#ifndef IPSNAPSHOT_H
#define IPSNAPSHOT_H

#include "BoolOpSeq.h"
#include <map>
struct Scope;


/**
*/
class IpSnapshot {
public:
    IpSnapshot();

    std::map<Inst  *,Expr     > rooms;
    std::map<Scope *,BoolOpSeq> conds;
    int date;

    static int cur_date;
};

#endif // IPSNAPSHOT_H
