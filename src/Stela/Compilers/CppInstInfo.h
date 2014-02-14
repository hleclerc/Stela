#ifndef CPPINSTINFO_H
#define CPPINSTINFO_H

#include "../Inst/Expr.h"
class CppCompiler;

/**
*/
class CppInstInfo {
public:
    struct Out {
        Out() : type( 0 ), num( -1 ) {}
        const BaseType *type;
        int num;
    };

    struct Writer {
        mutable CppInstInfo *info;
        mutable CppCompiler *cc;
        int nout;
    };

    struct DeclWriter : Writer {
        void write_to_stream( Stream &os ) const;
    };

    struct InstWriter : Writer {
        void write_to_stream( Stream &os ) const;
    };

    CppInstInfo( const Inst *inst ) : inst( inst ), out( Size(), inst->out_size() ) {
    }

    DeclWriter decl_writer( CppCompiler *cc, int nout );
    InstWriter inst_writer( CppCompiler *cc, int nout );

    const Inst   *inst;
    Vec<Out,-1,1> out;
};


#endif // CPPINSTINFO_H
