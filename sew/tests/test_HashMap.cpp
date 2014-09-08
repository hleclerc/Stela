#include <Stela/System/Stream.h>
#include <Stela/System/Assert.h>
#include <stdlib.h>
#include <vector>

/**
  Si on a la régularité statistique des clés, on peut envisager de faire des hash_map récursifs
  En fonction de la taille des buckets, on peut aussi faire
    -> lien vers autre niveau de HashMap
    -> ou lien vers liste chainée
    -> ou lien vers élément unique

  Objectif:
    -> stocker 1e6 clés + description de la localisation (local+maître). Caches=64ko puis 4Mo

  Pourquoi plusieurs petites tables que une grosse ?
    -> une grosse ne tient pas en cache L1
    -> mais de toutes façons, il faut passer par des données en L2... sauf si on va chercher

  Rq: si un inode n'est pas dans la table, il faut aller chercher "ailleurs", i.e.
    faire des requêtes collectives couteuses pour trouver le maître

  Rq: on peut aussi faire une table de hashage qui pointe sur des vecteurs triés...
    Histoire de profiter de la ligne de cache, on peut ajouter les données des vecteurs dans la table
*/
template<class Key,class Item,int nb_buckets=4096>
struct HashMap {
    enum {
        cache_line = 64, // bytes
    };

    struct Bucket {
        union TD {
            Key  *keys;
        };

        Bucket() : size( 0 ), rese( 0 ) {
        }

        ~Bucket() {
            if ( rese ) {
                //for( int n = 0; n < size; ++n )
                //    delete item( n );
                free( data );
            }
        }

        Key   *keys () { return reinterpret_cast<Key   *>(          data ); }
        Item **items() { return reinterpret_cast<Item **>( keys() + rese ); }

        Key   &key ( int n ) { return keys ()[ n ]; }
        Item *&item( int n ) { return items()[ n ]; }

        Item *find( Key k ) {
            int beg = 0, end = size;
            while ( end > beg ) {
                int mid = ( beg + end ) / 2;
                Key t = key( mid );
                if ( t == k )
                    return item( mid );
                if ( k < t )
                    end = mid;
                else
                    beg = mid + 1;
            }
            return 0;
        }

        bool add( Key k, Item *i ) {
            // no data ?
            if ( not rese ) {
                rese = 8;
                size = 1;
                data = (char *)malloc( rese * ( sizeof( Key ) + sizeof( Item * ) ) );
                item( 0 ) = i;
                key ( 0 ) = k;
                return true;
            }

            // else, find where to insert
            int beg = 0, end = size;
            while ( end > beg ) {
                int mid = ( beg + end ) / 2;
                Key t = key( mid );
                if ( k < t )
                    end = mid;
                else
                    beg = mid + 1;
            }

            // if there is enough room, add the new object
            if ( size < rese ) {
                for( int i = size; i > beg; --i ) {
                    item( i ) = item( i - 1 );
                    key ( i ) = key ( i - 1 );
                }
                item( beg ) = i;
                key ( beg ) = k;
                ++size;
                return true;
            }

            // else, allocate new room
            char *old = data;
            Key   *old_keys  = reinterpret_cast<Key   *>( old );
            Item **old_items = reinterpret_cast<Item **>( old_keys + rese );

            rese *= 2;
            data = (char *)malloc( rese * ( sizeof( Key ) + sizeof( Item * ) ) );
            for( int i = 0; i < beg; ++i ) {
                key ( i ) = old_keys [ i ];
                item( i ) = old_items[ i ];
            }
            key ( beg ) = k;
            item( beg ) = i;
            for( int i = beg; i < size; ++i ) {
                key ( i + 1 ) = old_keys [ i ];
                item( i + 1 ) = old_items[ i ];
            }
            ++size;

            free( old );
            return true;
        }

        bool rem( Key key ) {
            TODO;
            return false;
        }

        char  *data;
        int    size;
        int    rese;
        // -> 16 bytes

        // char  fill[ fill_len ];
    };

    HashMap() {
    }

    ~HashMap() {
    }

    Item *find( Key key ) {
        return buckets[ key & ( nb_buckets - 1 ) ].find( key );
    }

    /// return false if there is already a key `key`
    bool add( Key key, Item *item ) {
        return buckets[ key & ( nb_buckets - 1 ) ].add( key, item );
    }

    /// return false if there is no key `key`
    bool rem( Key key ) {
        return buckets[ key & ( nb_buckets - 1 ) ].rem( key );
    }

    // first level (-> hash map). Goal: stay in L1 cache
    Bucket buckets[ nb_buckets ];
};


struct Item {
    int data;
};

int main() {
    std::vector<int > keys ( 1000 );
    std::vector<Item> items( keys.size() );
    for( unsigned i = 0; i < keys.size(); ++i ) {
        keys [ i ] = rand() % ( keys.size() / 2 );
        items[ i ].data = keys [ i ];
    }

    HashMap<int,Item,16> hash_map;
    // std::vector<int> res( keys.size() );

    for( unsigned i = 0; i < keys.size(); ++i )
        hash_map.add( keys[ i ], &items[ i ] );

    for( unsigned i = 0; i < keys.size(); ++i )
        if ( Item *it = hash_map.find( keys[ i ] )  )
            ASSERT( keys[ i ] == it->data, "..." );
        else
            ERROR( "not found" );
}



