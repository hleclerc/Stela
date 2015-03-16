#include "SetVal.h"
#include "Room.h"
#include "Type.h"
#include "Var.h"
#include "Ip.h"

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
        if ( not var_list.size() )
            return IP_ERROR( "weird" );
        Ptr<Inst> old = var_list[ 0 ]->inst;
        Ptr<Inst> res = set_val( old, simplified( val ), 0, ip->cond_stack.back() );
        for( Var *v : var_list )
            v->inst = res;
    }
    virtual int size() const {
        return type->size();
    }
    virtual Ptr<Inst> forced_clone( Vec<Ptr<Inst> > &created ) const {
        return new Room( type );
    }
    virtual void write_1l_to( CodeGen_C *cc ) const {
        TODO;
    }

    Type *type; ///< creation type
    Vec<Var *> var_list;
};

Ptr<Inst> room( Type *type ) {
    return new Room( type );
}

