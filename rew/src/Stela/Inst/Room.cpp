#include "Select.h"
#include "Room.h"
#include "Ip.h"

static int nb_rooms = 0; ///< for the display
/**
  Pointer on some room...
*/
class Room : public Inst {
public:
    Room( int len, Expr val ) : val( val ), len( len ) {
        num = nb_rooms++;
    }
    Room( int len ) : len( len ) {
        num = nb_rooms++;
    }
    virtual void write_dot( Stream &os ) const {
        os << "(@" << num << ")" << val;
    }
    virtual void clone( Vec<Expr> &created ) const {
        TODO;
    }
    virtual int size() const {
        return len;
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        ERROR( "should not be here" );
        return 0;
    }
    virtual Expr _get_val() const {
        return val;
    }
    virtual void _set_val( Expr val ) {
        if ( ip->cond_stack.size() )
            this->val = select( ip->cur_cond(), val, this->val );
        else
            this->val = val;
    }
    virtual bool is_a_pointer() const {
        return true;
    }
    virtual Expr _at( int len ) {
        return val;
    }

    Expr val;
    int  len;
    int  num; ///< for the display
};

Expr room( int len, Expr val ) {
    return new Room( len, val );
}

Expr room( int len ) {
    return new Room( len );
}
