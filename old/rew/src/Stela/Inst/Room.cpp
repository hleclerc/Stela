#include "InstInfo_C.h"
#include "IpSnapshot.h"
#include "Select.h"
#include "FillAt.h"
#include "Store.h"
#include "Slice.h"
#include "Room.h"
#include "Cst.h"
#include "Ip.h"

static int nb_rooms = 0; ///< for the display
/**
  Pointer on some room...
*/
class Room : public Inst {
public:
    Room( int len, Expr val ) : date( IpSnapshot::cur_date ), val( val ), len( len ) {
        num = nb_rooms++;
    }
    Room( int len ) : date( IpSnapshot::cur_date ), val( cst( len, 0 ) ), len( len ) {
        num = nb_rooms++;
    }
    virtual void write_dot( Stream &os ) const {
        os << "@" << num;
    }
    virtual void write_to_stream( Stream &os, int prec = -1 ) const {
        os << "@" << num << "(" << val << ")";
    }
    virtual Expr forced_clone( Vec<Expr> &created ) const {
        return new Room( len );
    }
    virtual int size() const {
        return ip->type_ST->size();
    }
    virtual Expr _get_val( int len ) {
        if ( val->size() != len )
            return slice( val, 0, len );
        return val;
    }
    virtual void _set_val( Expr val, int len, Rese, Expr cond ) {
        if ( flags & CONST )
            return ip->disp_error( "attempting to modify a const value" );
        for( IpSnapshot *is : ip->snapshots ) {
            if ( date < is->date ) {
                if ( not is->changed.count( this ) )
                    is->changed[ this ] = this->val;
            }
        }
        if ( this->len != len ) {
            Expr n = fill_at( simplified( this->val ), simplified( val ), &ip->type_SI32, cst( 0 ) );
            if ( cond )
                this->val = select( cond, n, simplified( this->val ) );
            else
                this->val = n;
        } else {
            if ( cond )
                this->val = select( cond, simplified( val ), simplified( this->val ) );
            else
                this->val = simplified( val );
        }
    }
    virtual bool is_a_pointer() const {
        return true;
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
        IIC( this )->out_reg = cc->new_out_reg( IIC( this )->val_type );
    }
    virtual void write_to( Codegen_C *cc, int prec, OutReg *out_reg ) {
        *cc->os << "&" << *out_reg;
    }
    virtual void val_type_proposition( Type *type ) {
        IIC( this )->val_type = type;
    }
    virtual bool is_a_Room() const {
        return true;
    }
    virtual int size_ptd() const {
        return val->size();
    }

    Vec<Expr> future_dep;
    PI64 date;
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
