#include "InstInfo_C.h"
#include "Select.h"
#include "Store.h"
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
        os << "@" << num;
    }
    virtual void write_to_stream( Stream &os ) const {
        os << "(@" << num << ")" << val;
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new Room( len );
    }
    virtual int size() const {
        return ip->type_ST->size();
    }
    virtual Expr _get_val() {
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
    virtual void _visit_pointed_data( Visitor &v, bool want_dep ) {
        val->visit( v, true, want_dep );
    }
    virtual void _add_store_dep_if_necessary( Expr res, Expr fut ) {
        Expr st = store( this, val );
        res->add_dep( st );

        for( Expr d : future_dep )
            st->add_dep( d );
        future_dep.resize( 0 );
        future_dep << fut;
    }
    virtual void write_to( Codegen_C *cc, int prec ) {
        if ( prec < 0 )
            cc->on << *IIC( this )->val_type << " R" << ( IIC( this )->num_reg = cc->new_num_reg() ) << ";";
    }
    virtual void write_to( Codegen_C *cc, int prec, int num_reg ) {
        *cc->os << "&R" << num_reg;
    }
    virtual void val_type_proposition( Type *type ) {
        IIC( this )->val_type = type;
    }
    virtual bool is_a_Room() const {
        return true;
    }

    Vec<Expr> future_dep;
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
