#ifndef MEMCPY_H
#define MEMCPY_H

#include "TypeConfig.h"

void memcpy_bit( void *dst, int off_dst, const void *src, int off_src, int len );
void memcpy_bit( void *dst, const void *src, int len );


#endif // MEMCPY_H
