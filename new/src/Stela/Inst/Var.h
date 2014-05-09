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
    Var( Type *type, const Ptr<Inst> val ); ///<
    Var( Type *type =  0 ); ///< uninialized variable

    Var( Ref, const Var &var ); ///< reference

    Var( SI32 val );

    Var &operator=( const Var &var );

    Var ptr(); ///< pointer on this
    Var ptd(); ///< pointed data

    void write_to_stream( Stream &os ) const;
    int size() const;


    // attributes
    Ptr<Inst> inst;
    Type     *type;
    int       flags;

private:
    Var( const Var &var );
};

#endif // VAR_H
