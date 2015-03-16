#include "InstInfo_C.h"
#include "Ip.h"

InstInfo_C::InstInfo_C() {
    out_type = 0;
    val_type = &ip->type_Void;
    out_reg  = 0;
}
