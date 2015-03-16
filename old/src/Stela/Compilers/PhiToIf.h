#ifndef PHITOIF_H
#define PHITOIF_H

#include "../System/Vec.h"
class CppCompiler;
class CppInst;

/// convert phi expression to if with ifout and ifin instruction (something closer to sequential)
Vec<CppInst *> phi_to_if( const Vec<CppInst *> &outputs, CppCompiler *cc );

#endif // PHITOIF_H
