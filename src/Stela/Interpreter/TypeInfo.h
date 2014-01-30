#ifndef TYPEINFO_H
#define TYPEINFO_H

#include "../System/Vec.h"
#include "Var.h"
class ClassInfo;
class VarTable;

/**
*/
class TypeInfo {
public:
    struct Attr {
        int offset; ///< -2 means static, -1 means unknown (computed during run-time)
        int name;
        Var var;
    };

    TypeInfo( ClassInfo *orig );
    void parse_if_necessary();

    // input
    Vec<Var>        parameters;
    ClassInfo      *orig;
    Var             var;

    // output
    bool            parsed;
    SI32            static_size_in_bits; ///< -1 if dynamic
    SI32            static_alig_in_bits; ///< -1 if dynamic

    bool            has_a_destructor;

    Vec<TypeInfo *> ancestors;
    Vec<Attr>       dyna_attrs; ///< dynamic attributes
    Ptr<VarTable>   stat_attrs; ///< static attributes

    // context
    TypeInfo       *prev; ///< prev type in ClassInfo
};

#endif // TYPEINFO_H
