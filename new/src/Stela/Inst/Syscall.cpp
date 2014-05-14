#include "../CodeGen/InstInfo_C.h"
#include "../CodeGen/CodeGen_C.h"
#include "Syscall.h"
#include "Symbol.h"
#include "Op.h"
#include "Ip.h"

/**
   last child = condition
*/
class Syscall : public Inst {
public:
    Syscall() {}
    virtual void write_dot( Stream &os ) const {
        os << "syscall";
    }
    virtual int size() const {
        return inp.size() ? inp[ 0 ]->size() : 0;
    }
    virtual Ptr<Inst> forced_clone( Vec<Ptr<Inst> > &created ) const {
        return new Syscall;
    }
    virtual void set( Ptr<Inst> val ) {
        dep[ 0 ]->set( val );
    }
    virtual void write_1l_to( CodeGen_C *cc ) const {
        *cc->os << "syscall( ";
        for( int i = 0; i < inp.size() - 1; ++i )
            *cc->os << ( i ? ", " : "" ) << cc->code( inp[ i ] );

        *cc->os << " )";
    }
    virtual Type *out_type_proposition( CodeGen_C *cc ) const {
        return ip->type_ST;
    }
    virtual Type *inp_type_proposition( CodeGen_C *cc, int ninp ) const {
        return ninp < inp.size() - 1 ? ip->type_ST : &ip->type_Bool;
    }
    virtual void add_when_cond( const Ptr<Inst> &cond ) {
        Ptr<Inst> cond_exec = inp.back();
        Ptr<Inst> new_cond = op( &ip->type_Bool, &ip->type_Bool, cond, &ip->type_Bool, cond_exec, Op_and() );

        if ( IIC( this )->add_when_possibility( new_cond ) )
            return;

        // children that have to be executed in all the cases (under cond)
        cond_exec->add_when_cond( cond );
        for( Ptr<Inst> i : dep )
            i->add_when_cond( cond );

        //
        for( int i = 0; i < inp.size() - 1; ++i )
            inp[ i ]->add_when_cond( new_cond );
    }
    virtual void _remove_cond( Vec<Ptr<Inst> > &cr ) {
        Ptr<Inst> n = symbol( "x", 1 );
        mod_inp( inp.size() - 1, n );
        cr << n;
    }
};

Ptr<Inst> syscall( const Vec<Ptr<Inst> > &children, const Ptr<Inst> &cnd ) {
    Syscall *res = new Syscall;
    for( const Ptr<Inst> &ch : children )
        res->add_inp( ch );
    res->add_inp( cnd );
    return res;
}
