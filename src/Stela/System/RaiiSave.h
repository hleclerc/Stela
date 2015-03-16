#ifndef RAIISAVE_H
#define RAIISAVE_H

template<class T>
struct RaiiSave {
    RaiiSave( T &val, const T &nval ) : RaiiSave( val ) { val = nval; }
    RaiiSave( T &val ) : ref( val ), old( val ) {}
    ~RaiiSave() { ref = old; }
    T &ref, old;
};

template<class T>
RaiiSave<T> raii_save( T &val ) {
    return val;
}

template<class T>
RaiiSave<T> raii_save( T &val, const T &nval ) {
    return RaiiSave<T>( val, nval );
}


#endif // RAIISAVE_H
