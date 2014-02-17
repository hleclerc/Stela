#ifndef PHITOIF_H
#define PHITOIF_H

#include "../Inst/Inst.h"

/// convert phi expression to if with ifout and ifin instruction (something closer to sequential)
Vec<ConstPtr<Inst> > phi_to_if( const Vec<ConstPtr<Inst> > &outputs );

#endif // PHITOIF_H
