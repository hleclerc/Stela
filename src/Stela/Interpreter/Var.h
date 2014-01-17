#ifndef VAR_H
#define VAR_H

#include "PRef.h"
class Type;

/**
  Several variables may point to the same Ref, and they are also able to change the type of Ref,
  explaining the need for a double pointer
*/
class Var {
public:
    enum {
        WEAK_CONST = 1 ///< this variable should be changed, but nothing can guarantee that it won't be changed elsewhere
    };

    // Où stocker ip ??
    Var( Interpreter *ip, Var *type, const Expr &expr );
    Var( Interpreter *ip, Var *type );
    Var();

    ~Var();

    void write_to_stream( Stream &os ) const;
    operator bool() const { return type and data and data->ptr; }
    const PI8 *cst_data() const; ///< return 0 or a pointer on data if known
    bool referenced_more_than_one_time() const;

    Expr get() const;
    bool set( Expr expr );

    Ptr<PRef> data;
    Ptr<PRef> type;
    int       flags;
};

#endif // VAR_H
