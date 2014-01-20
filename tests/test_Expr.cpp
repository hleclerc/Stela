#include <Stela/Expr/PointerOn.h>
#include <Stela/Expr/Syscall.h>
#include <Stela/Expr/Concat.h>
#include <Stela/Expr/ValAt.h>
#include <Stela/Expr/Slice.h>
#include <Stela/Expr/Rand.h>
#include <Stela/Expr/Cst.h>
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

    Expr pa = pointer_on( tcs[ 0 ], 64 );
    Expr pb = pointer_on( tcs[ 0 ], 64 );
    PA( pa );
    PA( pb );

    PA( add( bt_SI32, cst( 10 ), cst( 20 ) ) );

    PA( rand( 64 ) );

    PA( slice( cst( SI64( 0x176548 ) ), 8, 32 ) );

    Expr struct_expr = concat( cst( 0x17 ), rand( 32 ) );
    PA( struct_expr );
    PA( slice( struct_expr,  0, 16 ) );
    PA( slice( struct_expr,  0, 32 ) );
    PA( slice( struct_expr, 32, 64 ) );
    PA( slice( struct_expr, 32, 63 ) );

    PA( val_at( pointer_on( cst( 0x32 ), 64 ), 32 ) );
    PA( pointer_on( val_at( cst( 0x32 ), 32 ), 32 ) );
}
