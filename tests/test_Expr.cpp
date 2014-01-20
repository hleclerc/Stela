#include <Stela/Expr/Syscall.h>
#include <Stela/Expr/Rand.h>
#include <Stela/Expr/Cst.h>
#include <Stela/Expr/Ptr.h>
#include <Stela/Expr/Op.h>

using namespace Expr_NS;

Stream &pp( Stream &os, Expr e ) { os << e << " at " << e.inst.ptr(); return os; }
#define PA( A ) pp( std::cout << "  " << #A << " -> ", A ) << std::endl


int main() {
    Expr tcs[] = { cst( 0 ), cst( 1 ) };
    PA( tcs[ 0 ] );
    PA( tcs[ 1 ] );
    PA( cst( 0 ) );
    PA( cst( 1 ) );

    Expr sa = syscall( cst( 6 ), 2, tcs, 32 ).ret;
    Expr sb = syscall( cst( 6 ), 2, tcs, 32 ).ret;
    PA( sa );
    PA( sb );

    Expr pa = ptr( tcs[ 0 ], 64 );
    Expr pb = ptr( tcs[ 0 ], 64 );
    PA( pa );
    PA( pb );

    PA( add( bt_SI32, cst( 10 ), cst( 20 ) ) );

    PA( rand( 64 ) );
}
