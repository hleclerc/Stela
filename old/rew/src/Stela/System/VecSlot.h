#ifndef VECSLOT_H
#define VECSLOT_H

/**
  G -> receiver object
  R -> a getter to get the receveir object (where stored)
*/
template<class T,class ROG>
struct VecSlot {
    struct Setter {
        operator const T &() const { return inl->operator[]( ind ); }
        void operator=( const T &expr ) { inl->_roge.; }
        VecSlot *inl;
        int      ind;
    };
    int size() const { return _size; }
    Inst *_inst() { return reinterpret_cast<Inst *>( reinterpret_cast<char*>( this ) - offsetof( Inst, inp ) ); }
    void resize( int ns ) { _inst()->_resize_inp( ns ); }
    const Expr &operator[]( int ind ) const;
    Setter operator[]( int ind ) { return Setter{ this, ind }; }


    R   _roge;
    T  *_data;
    int _size;
};


#endif // VECSLOT_H
