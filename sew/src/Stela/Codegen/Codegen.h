#ifndef CODEGEN_H
#define CODEGEN_H

#include "../System/StreamSep.h"
#include "../System/AutoPtr.h"
#include "../System/Stream.h"
#include "../Ssa/Inst.h"
#include <sstream>

/**
*/
class Codegen {
public:
    struct Writable {
        virtual ~Writable() {}
        virtual void write_to_stream( Stream &os ) const = 0;
    };
    struct WriteOut {
        void write_to_stream( Stream &os ) { cc->write_out( expr ); }
        Codegen *cc;
        Expr     expr;
    };

    Codegen();
    Codegen &operator<<( Expr inst );
    virtual void write_to( Stream &os ) = 0;
    virtual void exec() = 0;

    void write_out( Expr inst );

    void set_os( Stream *os, int nsp = -1 );

    virtual void write_beg_cast_bop( Type *type ) = 0;
    virtual void write_end_cast_bop( Type *type ) = 0;

    Vec<Expr> fresh;
    OutReg *new_reg( Type *type, String name = "" );

    StreamSepMaker<Stream> on;
    std::ostringstream     ms; ///< main stream
    Stream                *os; ///< currrent stream
    int                    num_reg;
};

#endif // CODEGEN_H
