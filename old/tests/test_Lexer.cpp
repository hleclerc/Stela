#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#ifdef METIL_COMP_DIRECTIVE
#pragma cpp_flag -std=c++11
#endif
using namespace std;
#define PRINT( A ) \
    std::cout << "  " << #A << " -> " << (A) << std::endl

template<class T>
std::ostream &operator<<( std::ostream &os, const vector<T> &v ) {
    for( int i = 0; i < v.size(); ++i )
        os << v[ i ] << " ";
    return os;
}


void getScan( string s, char c, vector<int> &S0, vector<int> &S1 ) {
    int n = s.size();
    S0.resize( 2 * n + 1 );
    S1.resize( 2 * n + 1 );
    for( int i = 0, t = 0, a = 0; ; ++i ) {
        S0[ i ] = t;
        S1[ i ] = a;
        if ( i == 2 * n - 1 )
            break;

        if ( s[ i % n ] == c )
            ++t;
        else
            a += t;
    }
}

int costToLeft( const vector<int> &S0, const vector<int> &S1, int beg, int end ) {
    return S1[ end ] - S1[ beg ] - ( end - beg - S0[ end ] + S0[ beg ] ) * S0[ beg ];
};

int cost( string s ) {
    int n = s.size();
    vector<int> S0, S1, U0, U1;
    getScan( s, 'R', S0, S1 );
    getScan( s, 'L', U0, U1 );
    PRINT( S0 );
    PRINT( S1 );

    // exploring all the possibilities for the middle point
    int best = 1e8;
    string sc = s + s;
    for( int m = 0; m < n; ++m ) {
        int beg = m, end = m + n;
        for( int mid : { m + n / 2, m + n / 2 + 1 } ) {
            int cost = costToLeft( S0, S1, beg, mid ) + costToLeft( U0, U1, mid, end );
            cout << string( sc.begin() + beg, sc.begin() + mid ) << " "
                 << string( sc.begin() + mid, sc.begin() + end ) << " " << cost << endl;
            best = min( best, cost );
        }
    }

    return best;
}

int main() {
    PRINT( cost( "LRLRLRLR" ) );
}



