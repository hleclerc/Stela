#include "Syscall.h"

/**
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
};

Ptr<Inst> syscall( const Vec<Ptr<Inst> > children ) {
    Syscall *res = new Syscall;
    for( const auto &ch : children )
        res->add_inp( ch );
    return res;
}
