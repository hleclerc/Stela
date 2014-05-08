#include "PointedData.h"

/**
*/
class PointedData : public Inst {
public:
    PointedData( int len ) : len( len ) {
    }
    virtual void write_dot( Stream &os ) const {
        os << "@";
    }
    virtual void write_to_stream( Stream &os ) const {
        os << "@" << inp[ 0 ];
    }
    virtual int size() const {
        return len;
    }
    virtual Ptr<Inst> _simplified() {
        if ( Ptr<Inst> ptr = inp[ 0 ]->_pointer_on( 0, len ) )
            return simplified( ptr );
        return 0;
    }

    int len;
};

Ptr<Inst> pointed_data( Ptr<Inst> ptr, int size ) {
    PointedData *res = new PointedData( size );
    res->add_inp( ptr );
    return res;
}
