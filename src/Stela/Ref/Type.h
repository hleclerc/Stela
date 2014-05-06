#ifndef TYPE_H
#define TYPE_H

#include "../System/Stream.h"
#include "../Inst/Expr.h"
#include "Var.h"

/**
*/
class Type {
public:
    Type( int name = -1 );

    int size(); ///< in bits
    bool pod() const;

    void write_to_stream( Stream &os ) const;

    void parse();

    // modification
    void add_room( int size );
    void add_field( int name, Var data );

    const BaseType *bt;
    int name;

protected:
    int _size;
};

#endif // TYPE_H
