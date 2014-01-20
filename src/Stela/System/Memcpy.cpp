#include "Memcpy.h"
#include "Assert.h"
#include <string.h>

void memcpy_bit( PI8 *dst, int off_dst, const PI8 *src, int off_src, int len ) {
    if ( int o = off_dst / 8 ) return memcpy_bit( dst + o, off_dst % 8, src, off_src, len );
    if ( int o = off_src / 8 ) return memcpy_bit( dst, off_dst, src + o, off_src % 8, len );

    if ( off_src % 8 or off_src % 8 or len % 8 )
        TODO;
    else
        memcpy( dst, src, len / 8 );
}

