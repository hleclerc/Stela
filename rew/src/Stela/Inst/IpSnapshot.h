#ifndef IPSNAPSHOT_H
#define IPSNAPSHOT_H

#include "Var.h"
#include <map>

/**
*/
class IpSnapshot {
public:
    IpSnapshot();

    PI64 date;
    static PI64 cur_date;

    std::map<Expr,Expr> changed; ///< changed variables (Room -> old value)
};

#endif // IPSNAPSHOT_H
