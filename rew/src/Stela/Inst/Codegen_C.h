#ifndef CODEGEN_C_H
#define CODEGEN_C_H

#include "../System/StreamSep.h"
#include "Codegen.h"

/**
*/
class Codegen_C : public Codegen {
public:
    Codegen_C();

    virtual void write_to( Stream &os );
    virtual void exec();

    StreamSepMaker<std::ostringstream> on;
    std::ostringstream main_os;
    std::ostringstream *os;

protected:
    void make_code();
    int nb_regs;
};

#endif // CODEGEN_C_H
