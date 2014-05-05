#ifndef REFITEM_H
#define REFITEM_H

#include "../System/Stream.h"

/**
*/
class RefItem {
public:
    RefItem();
    virtual ~RefItem();

    virtual void write_to_stream( Stream &os ) const;
};

#endif // REFITEM_H
