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
    enum { UNKNOWN_OFFSET = -1 };
    struct Attr {
        bool dynamic() const { return offset >= UNKNOWN_OFFSET; }
        bool stat() const { return offset < UNKNOWN_OFFSET; }

        int offset; ///< -2 means static, UNKNOWN_OFFSET means unknown (i.e. computed during run-time)
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
