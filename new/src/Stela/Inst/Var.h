#ifndef VAR_H
#define VAR_H

#include "Inst.h"
class Type;

/**
*/
class Var {
public:
    // flags
    enum {
        WEAK_CONST = 1
    };

    Var( Ref, Type *type, const Ptr<Inst> val ); ///<
    Var( Type *type, const Ptr<Inst> val ); ///< make a new room and set to val
    Var( Type *type =  0 ); ///< uninialized variable

    Var( SI32 val );

    Var &reassign( const Var &var );

    Var ptr(); ///< pointer on this
    Var ptd(); ///< pointed data

    void write_to_stream( Stream &os ) const;
    int size() const;


    // attributes
    Ptr<Inst> inst;
    Type     *type;
    int       flags;
};

Var syscall( Vec<Var> &inp );

#endif // VAR_H
