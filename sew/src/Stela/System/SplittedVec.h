#ifndef SPLITTEDVEC_H
#define SPLITTEDVEC_H

#include "../System/TensorOrder.h"
#include "../System/Assert.h"
#include <stdlib.h>
#include <string.h>
#include <utility>
#include <new>

///
template<class T,int atomic_size,int size_first_item=atomic_size,class ST=int>
class SplittedVec {
public:
    struct Item {
        Item *prev, *next;
        T *cur, *end;
        char *beg[ size_first_item * sizeof( T ) ];

        const T *ptr() const { return (const T *)beg; }
        T *ptr() { return (T *)beg; }

        const T *ptr( int n ) const { return (const T *)beg + n; }
        T *ptr( int n ) { return (T *)beg + n; }

        const T &val( int n ) const { return *ptr( n ); }
        T &val( int n ) { return *ptr( n ); }

        ST size() const { return cur - ptr(); }
    };
    static const int base_item_size = sizeof( Item ) - sizeof( size_first_item );

    SplittedVec() : last( &first ) {
        first.prev = 0;
        first.next = 0;
        first.cur = first.ptr();
        first.end = first.ptr() + size_first_item;
    }

    ~SplittedVec() {
        for( Item *i = last ; i; ) {
            for( const T *p = i->cur - 1; p >= i->ptr(); --p )
                p->~T();
            Item *o = i;
            i = i->prev;
            if ( o != &first )
                free( o ); // , reinterpret_cast<char *>( o->end ) - reinterpret_cast<char *>( o )
        }
    }

    struct Iterator {
        void operator++() {
            ++cur;
            while ( cur >= item->cur ) {
                if ( not item->next ) {
                    item = 0;
                    cur  = 0;
                    return;
                }
                item = item->next;
                cur  = item->ptr();
            }
        }
        T &operator*() {
            return *cur;
        }
        bool operator==( const Iterator &c ) const { return item == c.item and cur == c.cur; }

        Item *item;
        T    *cur;
    };

    struct ConstIterator {
        void operator++() {
            ++cur;
            while ( cur >= item->cur ) {
                if ( not item->next )
                    return;
                item = item->next;
                cur  = item->ptr();
            }
        }
        const T &operator*() const { return *cur; }
        bool operator==( const ConstIterator &c ) const { return item == c.item and cur == c.cur; }
        bool operator!=( const ConstIterator &c ) const { return item != c.item or  cur != c.cur; }

        const Item *item;
        const T    *cur;
    };

    Iterator begin() { return Iterator{ &first, first.ptr() }; }
    Iterator end  () { return Iterator{ last, last->cur }; }

    ConstIterator begin() const { return ConstIterator{ &first, first.ptr() }; }
    ConstIterator end  () const { return ConstIterator{ last, last->cur }; }


    void clear() {
        this->~SplittedVec();

        last = &first;
        first.prev = 0;
        first.next = 0;
        first.cur = first.ptr();
        first.end = first.ptr() + size_first_item;
    }

    template<class T2>
    SplittedVec &operator<<( const T2 &val ) {
        push_back( val );
        return *this;
    }

   T *push_back() {
        T *res = get_room_for();
        new( res ) T;
        return res;
    }

    template<class T0>
    T *push_back( const T0 &v0 ) {
        T *res = get_room_for();
        new( res ) T( v0 );
        return res;
    }

    template<class T0>
    T *push_back( T0 &&v0 ) {
        T *res = get_room_for();
        new( res ) T( std::move( v0 ) );
        return res;
    }

    template<class T0,class T1>
    T *push_back( const T0 &v0, const T1 &v1 ) {
        T *res = get_room_for();
        new( res ) T( v0, v1 );
        return res;
    }

    template<class T0,class T1,class T2>
    T *push_back( const T0 &v0, const T1 &v1, const T2 &v2 ) {
        T *res = get_room_for();
        new( res ) T( v0, v1, v2 );
        return res;
    }

    template<class T0,class T1,class T2,class T3>
    T *push_back( const T0 &v0, const T1 &v1, const T2 &v2, const T3 &v3 ) {
        T *res = get_room_for();
        new( res ) T( v0, v1, v2, v3 );
        return res;
    }

    template<class T0,class T1,class T2,class T3,class T4>
    T *push_back( const T0 &v0, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4 ) {
        T *res = get_room_for();
        new( res ) T( v0, v1, v2, v3, v4 );
        return res;
    }

    /// append val if not present and return pointer to new elem. Else, return pointer to elem which is equal to val
    template<class T2>
    T *push_back_unique( const T2 &val ) {
        for( ST i = 0; i < size(); ++i ) // TODO: optimize
            if ( operator[]( i ) == val )
                return &operator[]( i );
            return push_back( val );
    }

    template<class T2>
    T *insert( ST index, const T2 &val ) {
        if ( index == 0 )
            return push_back( val );
        push_back( std::move( back() ) );
        for( int i = size() - 3; i >= index; --i )
            operator[]( i + 1 ) = std::move( operator[]( i ) );
        return &( operator[]( index ) = val );
    }

    ///
    void pop_back() {
        --last->cur;
        last->cur->~T();
        if( last->cur == last->ptr() ) {
            Item *o = last;
            last = last->prev;
            if ( o != &first )
                free( o ); // , reinterpret_cast<char *>( o->end ) - reinterpret_cast<char *>( o )
            if ( not last )
                last = &first;
            last->next = 0;
        }
    }

    ///
    T pop_back_val() {
        T res = back();
        pop_back();
        return res;
    }

    /// TODO : optimize if POD
    void write( const T *d, ST s ) {
        for( int i = 0; i < s; ++i )
            operator<<( d[ i ] );
    }

    /// binary copy
    template<class R>
    void write_bin( const R &d ) {
        memcpy( get_room_for( sizeof( R ) / sizeof( T ) ), &d, sizeof( R ) );
    }

    template<class Op> void apply( Op &op ) const {
        for( const Item *i = &first; i; i = i->next )
            for( const T *p = i->ptr(); p < i->cur; ++p )
                op( *p );
    }

    template<class Op> void apply( Op &op ) {
        for( Item *i = &first; i; i = i->next )
            for( T *p = i->ptr(); p < i->cur; ++p )
                op( *p );
    }

    const T &back() const { return operator[]( size() - 1 ); }
    T &back() { return operator[]( size() - 1 ); }

    ST size() const {
        ST res = 0;
        for( const Item *i = &first; i; i = i->next )
            res += i->cur - i->ptr();
        return res;
    }

    void copy_to( T *d ) const {
        for( const Item *i = &first; i; i = i->next ) {
            ST s = i->cur - i->ptr();
            memcpy( d, i->beg, s );
            d += s;
        }
    }

    const T &operator[]( ST index ) const {
        ASSERT_IF_DEBUG( index >= 0 );
        for( const Item *i = &first; i; i = i->next ) {
            ST s = i->cur - i->ptr();
            if ( index < s )
                return i->val( index );
            index -= s;
        }
        ASSERT( 0, "index > size" );
        return *reinterpret_cast<T *>( 0 );
    }

    T &operator[]( ST index ) {
        ASSERT_IF_DEBUG( index >= 0 );
        for( Item *i = &first; i; i = i->next ) {
            ST s = i->cur - i->ptr();
            if ( index < s )
                return i->val( index );
            index -= s;
        }
        ASSERT( 0, "index > size" );
        return *(T *)first.beg;
    }

    T *get_room_for( ST nb_elements = 1 ) {
        if ( last == 0 or last->cur + nb_elements + 1 > last->end )
            return get_room_in_new_Item( nb_elements );
        //
        T *res = last->cur;
        last->cur += nb_elements;
        return res;
    }

    T *get_room_in_new_Item( ST nb_elements ) {
        Item *old_last = last;
        ST rese = base_item_size + sizeof( T ) * ( nb_elements <= atomic_size ? atomic_size : nb_elements );
        last = reinterpret_cast<Item *>( malloc( rese ) );
        last->cur = last->ptr() + nb_elements;
        last->end = reinterpret_cast<T *>( reinterpret_cast<char *>( last ) + rese );
        last->prev = old_last;
        last->next = 0;
        if ( old_last )
            old_last->next = last;

        return last->ptr();
    }

    void erase( int n ) {
        for( int i = n + 1; i < size(); ++i )
            operator[]( i - 1 ) = operator[]( i );
        pop_back();
    }

    Item *last, first;
};

template<class T,int s,int p,class ST>
struct TensorOrder<SplittedVec<T,s,p,ST> > {
    static const int res = 1;
};

#endif // SPLITTEDVEC_H
