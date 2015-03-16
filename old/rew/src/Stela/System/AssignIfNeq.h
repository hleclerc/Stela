#ifndef ASSIGNIFNEQ_H
#define ASSIGNIFNEQ_H

template<class T>
bool assign_if_neq( T &dst, const T &src ) {
    if ( not ( dst == src ) ) {
        dst = src;
        return true;
    }
    return false;
}

#endif // ASSIGNIFNEQ_H
