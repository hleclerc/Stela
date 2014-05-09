#include "SetVal.h"

/**
*/
class SetVal : public Inst {
public:
    virtual void write_dot( Stream &os ) const {
        os << "set_val";
        if ( offset )
            os << "[off=" << offset << "]";
    }
    virtual void write_to_stream( Stream &os ) const {
        write_dot( os );
        if ( inp.size() > 2 ) {
            os << "{" << inp[ 2 ];
            for( int i = 3; i < inp.size(); ++i )
                os << "," << inp[ i ];
            os << "}";
        }
        os << "(" << inp[ 0 ] << "," << inp[ 1 ] << ")";
    }
    virtual void set( Ptr<Inst> val ) {
        inp[ 0 ]->set( val );
    }
    virtual void add_var_ptr( Var *var ) {
        inp[ 0 ]->add_var_ptr( var );
    }

    virtual Ptr<Inst> _simplified() {
        if ( inp[ 0 ]->size() == inp[ 1 ]->size() )
            return simplified( inp[ 1 ] );
        return 0;
    }
    virtual Ptr<Inst> _pointer_on( int beg, int len ) {
        return inp[ 1 ]->_pointer_on( beg, len );
    }
    virtual int size() const {
        return inp[ 0 ]->size();
    }

    int offset;
};

Ptr<Inst> set_val( Ptr<Inst> src, Ptr<Inst> val, Vec<Ptr<Inst> > conds, int offset ) {
    SetVal *res = new SetVal;
    res->offset = offset;
    res->add_inp( src );
    res->add_inp( val );
    for( Ptr<Inst> c : conds )
        res->add_inp( c );
    return res;
}
