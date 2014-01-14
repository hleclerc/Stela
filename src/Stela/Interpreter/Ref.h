#ifndef REF_H
#define REF_H

#include "Expr.h"

/**
*/
class Ref : public ObjectWithCptUse {
public:
    Ref();
    virtual ~Ref();

    virtual Expr get() const = 0;
    // Ref *set( const Expr &expr ) = 0;
};

#endif // REF_H
