#ifndef TYPEPROMOTEINT_H
#define TYPEPROMOTEINT_H

#include "../System/TypeConfig.h"

template<class T1,class T2>
struct TypePromoteInt {};

template<> struct TypePromoteInt<Bool,Bool> { typedef Bool T; };
template<> struct TypePromoteInt<Bool,PI8 > { typedef PI8  T; };
template<> struct TypePromoteInt<Bool,SI8 > { typedef SI8  T; };
template<> struct TypePromoteInt<Bool,PI16> { typedef PI16 T; };
template<> struct TypePromoteInt<Bool,SI16> { typedef SI16 T; };
template<> struct TypePromoteInt<Bool,PI32> { typedef PI32 T; };
template<> struct TypePromoteInt<Bool,SI32> { typedef SI32 T; };
template<> struct TypePromoteInt<Bool,PI64> { typedef PI64 T; };
template<> struct TypePromoteInt<Bool,SI64> { typedef SI64 T; };

template<> struct TypePromoteInt<PI8 ,Bool> { typedef PI8  T; };
template<> struct TypePromoteInt<PI8 ,PI8 > { typedef PI8  T; };
template<> struct TypePromoteInt<PI8 ,SI8 > { typedef SI8  T; };
template<> struct TypePromoteInt<PI8 ,PI16> { typedef PI16 T; };
template<> struct TypePromoteInt<PI8 ,SI16> { typedef SI16 T; };
template<> struct TypePromoteInt<PI8 ,PI32> { typedef PI32 T; };
template<> struct TypePromoteInt<PI8 ,SI32> { typedef SI32 T; };
template<> struct TypePromoteInt<PI8 ,PI64> { typedef PI64 T; };
template<> struct TypePromoteInt<PI8 ,SI64> { typedef SI64 T; };

template<> struct TypePromoteInt<SI8 ,Bool> { typedef SI8  T; };
template<> struct TypePromoteInt<SI8 ,PI8 > { typedef SI8  T; };
template<> struct TypePromoteInt<SI8 ,SI8 > { typedef SI8  T; };
template<> struct TypePromoteInt<SI8 ,PI16> { typedef SI16 T; };
template<> struct TypePromoteInt<SI8 ,SI16> { typedef SI16 T; };
template<> struct TypePromoteInt<SI8 ,PI32> { typedef SI32 T; };
template<> struct TypePromoteInt<SI8 ,SI32> { typedef SI32 T; };
template<> struct TypePromoteInt<SI8 ,PI64> { typedef SI64 T; };
template<> struct TypePromoteInt<SI8 ,SI64> { typedef SI64 T; };

template<> struct TypePromoteInt<PI16,Bool> { typedef PI16 T; };
template<> struct TypePromoteInt<PI16,PI8 > { typedef PI16 T; };
template<> struct TypePromoteInt<PI16,SI8 > { typedef SI16 T; };
template<> struct TypePromoteInt<PI16,PI16> { typedef PI16 T; };
template<> struct TypePromoteInt<PI16,SI16> { typedef SI16 T; };
template<> struct TypePromoteInt<PI16,PI32> { typedef PI32 T; };
template<> struct TypePromoteInt<PI16,SI32> { typedef SI32 T; };
template<> struct TypePromoteInt<PI16,PI64> { typedef PI64 T; };
template<> struct TypePromoteInt<PI16,SI64> { typedef SI64 T; };

template<> struct TypePromoteInt<SI16,Bool> { typedef SI16 T; };
template<> struct TypePromoteInt<SI16,PI8 > { typedef SI16 T; };
template<> struct TypePromoteInt<SI16,SI8 > { typedef SI16 T; };
template<> struct TypePromoteInt<SI16,PI16> { typedef SI16 T; };
template<> struct TypePromoteInt<SI16,SI16> { typedef SI16 T; };
template<> struct TypePromoteInt<SI16,PI32> { typedef SI32 T; };
template<> struct TypePromoteInt<SI16,SI32> { typedef SI32 T; };
template<> struct TypePromoteInt<SI16,PI64> { typedef SI64 T; };
template<> struct TypePromoteInt<SI16,SI64> { typedef SI64 T; };

template<> struct TypePromoteInt<PI32,Bool> { typedef PI32 T; };
template<> struct TypePromoteInt<PI32,PI8 > { typedef PI32 T; };
template<> struct TypePromoteInt<PI32,SI8 > { typedef SI32 T; };
template<> struct TypePromoteInt<PI32,PI16> { typedef PI32 T; };
template<> struct TypePromoteInt<PI32,SI16> { typedef SI32 T; };
template<> struct TypePromoteInt<PI32,PI32> { typedef PI32 T; };
template<> struct TypePromoteInt<PI32,SI32> { typedef SI32 T; };
template<> struct TypePromoteInt<PI32,PI64> { typedef PI64 T; };
template<> struct TypePromoteInt<PI32,SI64> { typedef SI64 T; };

template<> struct TypePromoteInt<SI32,Bool> { typedef SI32 T; };
template<> struct TypePromoteInt<SI32,PI8 > { typedef SI32 T; };
template<> struct TypePromoteInt<SI32,SI8 > { typedef SI32 T; };
template<> struct TypePromoteInt<SI32,PI16> { typedef SI32 T; };
template<> struct TypePromoteInt<SI32,SI16> { typedef SI32 T; };
template<> struct TypePromoteInt<SI32,PI32> { typedef SI32 T; };
template<> struct TypePromoteInt<SI32,SI32> { typedef SI32 T; };
template<> struct TypePromoteInt<SI32,PI64> { typedef SI64 T; };
template<> struct TypePromoteInt<SI32,SI64> { typedef SI64 T; };

template<> struct TypePromoteInt<PI64,Bool> { typedef PI64 T; };
template<> struct TypePromoteInt<PI64,PI8 > { typedef PI64 T; };
template<> struct TypePromoteInt<PI64,SI8 > { typedef SI64 T; };
template<> struct TypePromoteInt<PI64,PI16> { typedef PI64 T; };
template<> struct TypePromoteInt<PI64,SI16> { typedef SI64 T; };
template<> struct TypePromoteInt<PI64,PI32> { typedef PI64 T; };
template<> struct TypePromoteInt<PI64,SI32> { typedef SI64 T; };
template<> struct TypePromoteInt<PI64,PI64> { typedef PI64 T; };
template<> struct TypePromoteInt<PI64,SI64> { typedef SI64 T; };

template<> struct TypePromoteInt<SI64,Bool> { typedef SI64 T; };
template<> struct TypePromoteInt<SI64,PI8 > { typedef SI64 T; };
template<> struct TypePromoteInt<SI64,SI8 > { typedef SI64 T; };
template<> struct TypePromoteInt<SI64,PI16> { typedef SI64 T; };
template<> struct TypePromoteInt<SI64,SI16> { typedef SI64 T; };
template<> struct TypePromoteInt<SI64,PI32> { typedef SI64 T; };
template<> struct TypePromoteInt<SI64,SI32> { typedef SI64 T; };
template<> struct TypePromoteInt<SI64,PI64> { typedef SI64 T; };
template<> struct TypePromoteInt<SI64,SI64> { typedef SI64 T; };

#endif // TYPEPROMOTEINT_H
