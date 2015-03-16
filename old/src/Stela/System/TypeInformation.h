#ifndef TYPEINFORMATION_H
#define TYPEINFORMATION_H

template<class T>
struct TypeInformation {
    enum {
        type_prec = 0,
        float_type = 0
    };
};

template<>
struct TypeInformation<float> {
    enum {
        type_prec = 10,
        float_type = 32
    };
};

template<>
struct TypeInformation<double> {
    enum {
        type_prec = 11,
        float_type = 64
    };
};

template<>
struct TypeInformation<long double> {
    enum {
        type_prec = 12,
        float_type = 80
    };
};

template<int n,class T0,class T1> struct AltType { };
template<class T0,class T1> struct AltType<0,T0,T1> { typedef T0 T; };
template<class T0,class T1> struct AltType<1,T0,T1> { typedef T1 T; };

template<class T0,class T1>
struct TypePromote {
    enum { tp = int( TypeInformation<T0>::type_prec ) < int( TypeInformation<T1>::type_prec ) };
    typedef typename AltType<tp,T0,T1>::T T;
};

#endif // TYPEINFORMATION_H
