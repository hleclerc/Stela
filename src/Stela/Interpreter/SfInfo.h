#ifndef SFINFO_H
#define SFINFO_H

#include "../System/Vec.h"

/**
*/
struct SfInfo {
    const char *filename;
    Vec<int>    nstr_cor;
    const PI8  *tok_data;
};
#endif // SFINFO_H
