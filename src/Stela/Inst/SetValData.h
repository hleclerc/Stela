#ifndef SETVALDATA_H
#define SETVALDATA_H

struct SetValData {
    int off_size_in_bits; ///< size in bits of offset
    int size_is_in_bytes; ///< true if offset is expressed in bytes (false -> bits)
};

#endif // SETVALDATA_H
