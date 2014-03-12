#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#ifdef METIL_COMP_DIRECTIVE
#pragma cpp_flag -std=c++11
#endif
using namespace std;
typedef vector<int> V;
const int GMAX = 30000;

ostream &operator<<( ostream &os, const V &v ) {
    for( unsigned i = 0; i < v.size(); ++i )
        os << v[ i ] << " ";
    return os;
}

int fact( int k ) {
    return k > 1 ? fact( k - 1 ) * k : 1;
}

int c( int n, int k ) {
    return fact( k ) / ( fact( n ) * fact( k - n ) );
}

void make_poss( int poss[][ GMAX ], V vals ) {
    for( int x = 0; x < GMAX; ++x )
        poss[ 0 ][ x ] = x == 0;
    for( unsigned i = 0; i < vals.size(); ++i ) {
        for( int x = 0; x < GMAX; ++x )
            poss[ i + 1 ][ x ] = 0;
        for( unsigned j = 0; j <= i; ++j )
            for( int x = vals[ i ]; x < GMAX; ++x )
                poss[ i + 1 ][ x ] += poss[ j ][ x - vals[ i ] ];
    }
}

struct Toto {
    void f() {
        V vals = { 7,7,8,9,10,11,1,2,2,3,4,5,6 };

        std::sort( vals.begin(), vals.end() );
        std::cout << vals << std::endl;
        make_poss( poss_a, vals );

        for( unsigned i = 0; i <= vals.size(); ++i ) {
            std::cout << ( i ? vals[ i - 1 ] : 0 ) << " -> ";
            for( int x = 0; x < 10; ++x )
                std::cout << poss_a[ i ][ x ] << " ";
            std::cout << "\n";
        }
        std::cout << "\n";

        reverse( vals.begin(), vals.end() );
        std::cout << vals << std::endl;
        make_poss( poss_b, vals );

        for( unsigned i = 0; i <= vals.size(); ++i ) {
            std::cout << ( i ? vals[ i - 1 ] : 0 ) << " -> ";
            for( int x = 0; x < 10; ++x )
                std::cout << poss_b[ i ][ x ] << " ";
            std::cout << "\n";
        }
        std::cout << "\n";

        long int res = 0;
        reverse( vals.begin(), vals.end() );
        for( unsigned a = 0; a < vals.size(); ++a ) {
            for( unsigned b = 0; b < vals.size() - a - 1; ++b ) {
                int n = 0;
                for( int i = 0; i <= a; ++i )
                    n += vals[ i ] == vals[ a ];
                int k = n;
                for( int i = 0; i <= b; ++i )
                    k += vals[ vals.size() - 1 - i ] == vals[ a ];
                for( int s = 1; s < GMAX; ++s )
                    res += poss_a[ a + 1 ][ s ] * poss_b[ b + 1 ][ s ] * c( n, k );
            }
        }

        std::cout << vals << "\n";
        std::cout << res << "\n";
    }
    int poss_a[ 30 ][ GMAX ];
    int poss_b[ 30 ][ GMAX ];
};

int main() {
    Toto *t = new Toto;
    t->f();
}



