#include <unistd.h>   // getcwd
#include <string.h>   // getcwd
#include "GetCwd.h"

String get_cwd() {
    for( ST rese = 128; ; rese *= 2 ) {
        String res( rese, ' ' );
        if ( getcwd( &res[ 0 ], rese ) ) {
            res.resize( strlen( res.data() ) );
            return res;
        }
    }
    return String();
}
