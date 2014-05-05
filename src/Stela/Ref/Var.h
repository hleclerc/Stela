#ifndef VAR_H
#define VAR_H

#include "Ref.h"

/**
*/
class Var {
public:
    enum {
        WEAK_CONST = 1 // local constness (several Var can point on a single Ref)
    };

    Var( Type *type, int size, PI8 *data = 0, PI8 *knwn = 0 );
    Var();

    static Var from_val( int val );


    void write_to_stream( Stream &os ) const;

    // void operator=( int val );

protected:
    Ptr<Ref> ref;
    int      flags;
};

#endif // VAR_H
