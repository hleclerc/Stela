#include "SetVal.h"
#include "Room.h"
#include "Type.h"
#include "Var.h"

/**
*/
class Room : public Inst {
public:
    Room( Type *type ) : type( type ) {
    }
    virtual void write_dot( Stream &os ) const {
        os << "room[" << *type << "]";
    }
    virtual void add_var_ptr( Var *var ) {
        var_list << var;
    }
    virtual void rem_var_ptr( Var *var ) {
        var_list.remove_first_unordered( var );
    }
    virtual void set( Ptr<Inst> val ) {
        Ptr<Inst> res = set_val( this, simplified( val ), 0 );
        for( Var *v : var_list )
            v->inst = res;
    }

    Type *type; ///< creation type
    Vec<Var *> var_list;
};

Ptr<Inst> room( Type *type ) {
    return new Room( type );
}

