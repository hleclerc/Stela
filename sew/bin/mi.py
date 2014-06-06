import os, sys

def r( s ):
    return s.replace( "NAME", sys.argv[ 1 ] ).replace( "NLME", sys.argv[ 1 ].lower() )

h = r( """#ifndef STELA_INST_NAME_H
#define STELA_INST_NAME_H

#include "Inst.h"

Inst *NLME();

#endif // STELA_INST_NAME_H
""" )

c = r( """#include "NAME.h"

/**
*/
struct NAME : Inst {
    virtual void write_dot( Stream &os ) { os << "NAME"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new NAME; }
    virtual Type *type() { return 0; }
};

Inst *NLME() {
    NAME *res = new NAME;
    return res;
}
""" )


h_file = file( r( "src/Stela/Inst/NAME.h" ), "w" )
print >> h_file, h

c_file = file( r( "src/Stela/Inst/NAME.cpp" ), "w" )
print >> c_file, c

os.system( "./update_files.sh > /dev/null 2> /dev/null" )
