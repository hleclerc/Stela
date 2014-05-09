#ifndef CODEGEN_C_H
#define CODEGEN_C_H

#include "../System/StreamSep.h"
#include "CodeGen.h"

/**
*/
class CodeGen_C : public CodeGen {
public:
    CodeGen_C();

    virtual void write_to( Stream &os );
    virtual void exec();

protected:
    void make_code();

    StreamSepMaker<std::ostringstream> on;
    std::ostringstream main_os;
    std::ostringstream *os;

};

#endif // CODEGEN_C_H
