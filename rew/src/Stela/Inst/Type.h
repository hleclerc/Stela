#ifndef TYPE_H
#define TYPE_H

#include "../System/Stream.h"
#include "Var.h"
class SourceFile;
class Class;

/**
*/
class Type {
public:
    struct Attr {
        bool dyn() const { return offset >= 0; }
        bool sta() const { return offset <  0; }

        int offset; ///< in bits
        int name;
        Var var;

        SourceFile *sf;
        int off; ///< on sourcefile
    };

    Type( int name = -1 );

    void write_to_stream( Stream &os ) const;
    void write_C_decl( Stream &out ) const;

    int pod() const;
    int size() const;
    int alig() const;
    virtual void parse() const;
    const Attr *find_attr( int name ) const;
    void find_attr( Vec<const Attr *> &res, int name );
    Var make_attr( Var self, const Attr *attr ) const;

    int                 name;
    Class              *orig;
    Vec<Var>            parameters; ///< template parameters

    mutable int         _len;
    mutable int         _ali;
    mutable int         _pod;
    mutable bool        _parsed;
    mutable Vec<Attr>   _attributes; ///< dynamic and static attributes
    mutable bool        _has_a_destructor;
    mutable Vec<Type *> _ancestors;
};

#endif // TYPE_H
