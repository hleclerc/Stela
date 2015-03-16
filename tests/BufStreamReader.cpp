#include "BufStreamReader.h"

BufStreamReader::BufStreamReader( Celo::Ptr<Celo::Buffer> beg, int off ) : beg( beg ), off( off ) {
    update_beg();
}

