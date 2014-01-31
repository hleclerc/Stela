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

    const Attr *find_attr( int name );
    void find_attr( Vec<const Attr *> &res, int name );
    Var make_attr( const Var &self, const Attr *attr );

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
    Vec<Attr>       attributes; ///< dynamic and static attributes

    // context
    TypeInfo       *prev; ///< prev type in ClassInfo
};

#endif // TYPEINFO_H
