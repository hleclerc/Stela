#include "NstrCor.h"

NstrCor::NstrCor() {
    for( int i = 0; i < NB_USUAL_STRINGS; ++i )
        num( String( usual_strings_str[ i ], usual_strings_str[ i ] + usual_strings_len[ i ] ) );
}

String NstrCor::str( int num ) const {
    return num >= 0 ? num_to_str[ num ] : "Undefined nstring";
}

int NstrCor::num( String str ) {
    auto iter = str_to_num.find( str );
    if ( iter != str_to_num.end() )
        return iter->second;
    //
    int res = num_to_str.size();
    str_to_num[ str ] = res;
    num_to_str << str;
    return res;
}
