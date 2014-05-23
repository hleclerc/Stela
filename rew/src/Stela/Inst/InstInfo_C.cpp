#include "InstInfo_C.h"
#include "Ip.h"

InstInfo_C::InstInfo_C() : block( 0 ) {
    num_reg = -1;
    out_type = 0;
    val_type = &ip->type_Void;
    out_reg  = 0;
}
