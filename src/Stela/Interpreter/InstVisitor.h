#ifndef INSTVISITOR_H
#define INSTVISITOR_H

class Syscall;
class Rand;
class Inst;
class Cst;

/**
*/
struct InstVisitor {
    virtual void operator()( const Inst    &inst ) {} // default

    virtual void operator()( const Syscall &inst ) { operator()( reinterpret_cast<const Inst &>( inst ) ); }
    virtual void operator()( const Rand    &inst ) { operator()( reinterpret_cast<const Inst &>( inst ) ); }
    virtual void operator()( const Cst     &inst ) { operator()( reinterpret_cast<const Inst &>( inst ) ); }
};

#endif // INSTVISITOR_H
