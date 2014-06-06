#ifndef NSTRCOR_H
#define NSTRCOR_H

#include "../System/UsualStrings.h"
#include "../System/Stream.h"
#include "../System/Vec.h"
#include <map>

/**
*/
class NstrCor {
public:
    NstrCor(); ///< make usual strings
    String str( int num ) const;
    int num( String str );

protected:
    Vec<String> num_to_str;
    std::map<String,int> str_to_num;
};


#endif // NSTRCOR_H
