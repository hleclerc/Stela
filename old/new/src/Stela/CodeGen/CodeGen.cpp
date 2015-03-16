#include "CodeGen.h"

CodeGen::CodeGen() {
}

CodeGen &CodeGen::operator<<( ConstPtr<Inst> inst ) {
    fresh << inst;
    return *this;
}
