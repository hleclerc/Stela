#ifndef IPSNAPSHOT_H
#define IPSNAPSHOT_H

#include "Inst.h"
#include <map>
struct Scope;


/**
*/
class IpSnapshot {
public:
    IpSnapshot();

    std::map<Inst  *,Expr> rooms;
    std::map<Scope *,Expr> conds;
    int date;

    static int cur_date;
};

#endif // IPSNAPSHOT_H
