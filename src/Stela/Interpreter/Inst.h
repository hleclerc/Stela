#ifndef INST_H
#define INST_H

#include "../System/Stream.h"
#include "../System/Ptr.h"
#include "../System/Vec.h"
#include <cstddef>
struct Expr;

#ifdef METIL_COMP_DIRECTIVE
#pragma cpp_flag -Wno-invalid-offsetof
#endif

/**
*/
class Inst : public ObjectWithCptUse {
public:
    // types
    typedef Expr  Inp;
    typedef Inst *Ext;

    struct Out {
        struct Item {
            Inst *inst;
            int   ninp;
        };
        Vec<Item,-1,1> parents;
    };

    struct OutList {
        int size() const { return _size; }
        Inst *_inst() { return reinterpret_cast<Inst *>( reinterpret_cast<char*>( this ) - offsetof( Inst, out ) ); }

        Out *_data;
        int  _size;
    };

    struct InpList {
        struct Setter {
            operator const Expr &() const { return inl->operator[]( ind ); }
            void operator=( const Expr &expr );
            InpList *inl;
            int      ind;
        };
        int size() const { return _size; }
        Inst *_inst() { return reinterpret_cast<Inst *>( reinterpret_cast<char*>( this ) - offsetof( Inst, inp ) ); }
        void resize( int ns ) { _inst()->_resize_inp( ns ); }
        const Expr &operator[]( int ind ) const;
        Setter operator[]( int ind ) { return Setter{ this, ind }; }

        Inp *_data;
        int  _size;
    };

    struct ExtList {
        int size() const { return _size; }
        Inst *_inst() { return reinterpret_cast<Inst *>( reinterpret_cast<char*>( this ) - offsetof( Inst, ext ) ); }

        Ext *_data;
        int  _size;
    };

    // methods
    Inst();
    virtual ~Inst();

    virtual const PI8 *cst_data( int nout ) const;
    virtual void write_to_stream( Stream &os ) const = 0;

    // attributes
    OutList out;
    InpList inp;
    ExtList ext;

protected:
    virtual void _resize_inp( int ns ) = 0;
};

#endif // INST_H
