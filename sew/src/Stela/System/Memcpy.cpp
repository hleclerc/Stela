#include "Memcpy.h"
#include "Assert.h"
#include <string.h>

void memcpy_bit( void *dst, int off_dst, const void *src, int off_src, int len ) {
    if ( int o = off_dst / 8 ) return memcpy_bit( (PI8*)dst + o, off_dst % 8, (PI8*)src, off_src, len );
    if ( int o = off_src / 8 ) return memcpy_bit( (PI8*)dst, off_dst, (PI8*)src + o, off_src % 8, len );

    if ( off_src % 8 or off_src % 8 )
        TODO;
    else
        memcpy( dst, src, ( len + 7 ) / 8 );
}

void memset_bit( void *dst, int off_dst, bool val, int len ) {
    if ( int o = off_dst / 8 ) {
        dst = (PI8 *)dst + o;
        off_dst %= 8;
    }
    if ( val ) {
        if ( off_dst ) {
            if ( off_dst + len < 8 ) {
                TODO; // dst[ 0 ] |= 0xFF >> off_dst;
                return;
            }
            TODO; // dst[ 0 ] |= 0xFF;
            dst = (PI8 *)dst + 1;
            len -= off_dst;
        }
        for( ; len >= 8; len -= 8, dst = (PI8 *)dst + 1 )
            *(PI8 *)dst = 0xFF;
        if ( len )
            TODO;
    } else {
        if ( off_dst ) {
            if ( off_dst + len < 8 ) {
                TODO; // dst[ 0 ] &= 0xFF >> off_dst;
                return;
            }
            TODO; // dst[ 0 ] &= 0xFF;
            dst = (PI8 *)dst + 1;
            len -= off_dst;
        }
        for( ; len >= 8; len -= 8, dst = (PI8 *)dst + 1 )
            *(PI8 *)dst = 0x00;
        if ( len )
            TODO;
    }
}

void memcpy_bit( void *dst, const void *src, int len ) {
    return memcpy_bit( dst, 0, src, 0, len );
}
