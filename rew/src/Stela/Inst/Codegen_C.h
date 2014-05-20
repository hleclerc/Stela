#ifndef CODEGEN_C_H
#define CODEGEN_C_H

#include "../System/StreamSep.h"
#include "Codegen.h"
struct CBlockAsm;

/**
*/
class Codegen_C : public Codegen {
public:
    struct C_Code {
        void write_to_stream( Stream &os ) const;
        mutable Codegen_C *cc;
        mutable Expr       inst;
        int                prec;
    };

    Codegen_C();

    virtual void write_to( Stream &os );
    virtual void exec();

    C_Code code( Expr inst, int prec = 0 );
    int new_num_reg();

    StreamSepMaker<std::ostringstream> on;
    std::ostringstream main_os;
    std::ostringstream *os;

protected:
    void write( CBlockAsm &cba );
    void make_code();
    int nb_regs;
};

#endif // CODEGEN_C_H
