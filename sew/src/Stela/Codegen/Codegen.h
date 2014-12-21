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

    Codegen();
    Codegen &operator<<( Expr inst );
    virtual void write_to( Stream &os ) = 0;
    virtual void exec() = 0;

    void set_os( Stream *os, int nsp = -1 );
    virtual AutoPtr<Writable> var_decl( OutReg *reg ) = 0;

    Vec<Expr> fresh;
    OutReg *new_reg();

    StreamSepMaker<Stream> on;
    std::ostringstream     ms; ///< main stream
    Stream                *os; ///< currrent stream
    int                    num_reg;
};

#endif // CODEGEN_H
