var socadb = require('./PI8.js');
var asm = require('./asm.js/lib/asm.js');

//console.log( socadb.PI8 );
//n = new socadb.PI8;
//console.log( n.size_in_bits() );

function PI8( stdlib, foreign, buffer ) {
    "use asm";

    function geometricMean( start, end ) {
        start = start|0;
        end = end|0;

        return ( end - start ) | 0;
    }

    return { pouet: geometricMean };
}

var heap = new ArrayBuffer( 0x10000 );   // 64k heap
var fast = PI8( global, null, heap );    // produce exports object linked to AOT-compiled code
var resu = fast.pouet( 20, 30 ); // computes geometric mean of input values
console.log( resu );

try {
    report = asm.validate( String( PI8 ) );
    console.log( 'Validated!' );
} catch( e ) {
    console.error( e.stack );
    console.error( e.src );
    console.error( e.loc );
}
