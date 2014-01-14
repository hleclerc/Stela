#ifndef DEFOPERATIONS_H
#define DEFOPERATIONS_H

#include "../Ir/Struct_Operations.h"
#include "../System/Math.h"

template<class Op>
struct DefOperation {
    template<class T0,class T1,class T2>
    static bool op( T0 &r, T1 a, T2 b ) { return false; }
    template<class T0,class T1>
    static bool op( T0 &r, T1 a ) { return false; }
};

// unary
template<> struct DefOperation<Op_Not> {
    template<class T0,class T1> static bool op( T0 &r, T1 a ) { r = not a; return true; }
};

template<> struct DefOperation<Op_Log> {
    template<class T0,class T1> static bool op( T0 &r, T1 a ) { r = log( a ); return true; }
};

template<> struct DefOperation<Op_Ceil> {
    template<class T0,class T1> static bool op( T0 &r, T1 a ) { r = ceil( a ); return true; }
};

template<> struct DefOperation<Op_Conv> {
    template<class T0,class T1> static bool op( T0 &r, T1 a ) { return conv( r, a ); }
};

// binary
template<> struct DefOperation<Op_Or> {
    template<class T0,class T1,class T2> static bool op( T0 &r, T1 a, T2 b ) { r = a or b; return true; }
};

template<> struct DefOperation<Op_And> {
    template<class T0,class T1,class T2> static bool op( T0 &r, T1 a, T2 b ) { r = a and b; return true; }
};

template<> struct DefOperation<Op_Add> {
    template<class T0,class T1,class T2> static bool op( T0 &r, T1 a, T2 b ) { r = a + b; return true; }
};

template<> struct DefOperation<Op_Sub> {
    template<class T0,class T1,class T2> static bool op( T0 &r, T1 a, T2 b ) { r = a - b; return true; }
};

template<> struct DefOperation<Op_Mul> {
    template<class T0,class T1,class T2> static bool op( T0 &r, T1 a, T2 b ) { r = a * b; return true; }
};

template<> struct DefOperation<Op_Div> {
    template<class T0,class T1,class T2> static bool op( T0 &r, T1 a, T2 b ) { r = a / b; return true; }
};

template<> struct DefOperation<Op_Pow> {
    template<class T0,class T1,class T2> static bool op( T0 &r, T1 a, T2 b ) { r = pow( a, b ); return true; }
};

template<> struct DefOperation<Op_Mod> {
    template<class T0,class T1,class T2> static bool op( T0 &r, T1 a, T2 b ) { r = mod( a, b ); return true; }
};

template<> struct DefOperation<Op_Equ> {
    template<class T0,class T1,class T2> static bool op( T0 &r, T1 a, T2 b ) { r = a == b; return true; }
};

template<> struct DefOperation<Op_Neq> {
    template<class T0,class T1,class T2> static bool op( T0 &r, T1 a, T2 b ) { r = a != b; return true; }
};

template<> struct DefOperation<Op_Sup> {
    template<class T0,class T1,class T2> static bool op( T0 &r, T1 a, T2 b ) { r = a > b; return true; }
};

template<> struct DefOperation<Op_Inf> {
    template<class T0,class T1,class T2> static bool op( T0 &r, T1 a, T2 b ) { r = a < b; return true; }
};

template<> struct DefOperation<Op_SupEq> {
    template<class T0,class T1,class T2> static bool op( T0 &r, T1 a, T2 b ) { r = a >= b; return true; }
};

template<> struct DefOperation<Op_InfEq> {
    template<class T0,class T1,class T2> static bool op( T0 &r, T1 a, T2 b ) { r = a <= b; return true; }
};



#endif // DEFOPERATIONS_H
