#ifndef ARCH_H
#define ARCH_H

class BaseType;
class Expr;

/**
*/
class Arch {
public:
    Arch();

    Expr            cst_ptr( SI64 val ); ///< make a cst, with size == ptr_size
    const BaseType *bt_ptr(); ///< base type to represent a pointer

    int             ptr_size; ///< ptr size in bits
    int             ptr_alig;
    bool            rev_endn; ///< reverse endianness
};

extern Arch *arch; ///< current global architecture

#endif // ARCH_H
