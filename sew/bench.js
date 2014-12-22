//var socadb = require('./PI8.js');
//var asm = require('./asm.js/lib/asm.js');

/// asm modules
/// manages the asm function + associated heap
/// (if the heap is resized, functions are re-compiled)
am = {
    modules    : [],
    functions  : {},
    heap       : new ArrayBuffer( 0x10000000 ),
    global     : typeof window === 'undefined' ? global : window,
    heap_size  : function() { return this.heap.byteLength; },
    push       : function( module ) {
        this.modules.push( module );
        var res = module( this.global, null, this.heap );
        for( var v in res ) {
            this.functions[ v ] = res[ v ];
        }
    },
    resize_heap: function( new_size ) {
        // a new heap
        old_heap = this.heap;
        this.heap = new ArrayBuffer( new_size );
        new Uint8Array( this.heap, 0, old_heap.byteLength ).set( new Uint8Array( old_heap ) );
        // re-compilation
        for( var num_module = 0; num_module < this.modules.length; num_module++ ) {
            module = this.modules[ num_module ];
            var res = module( this.global, null, this.heap );
            for( var v in res ) {
                this.functions[ v ] = res[ v ];
            }
        }
    }
};

am.push( function( stdlib, foreign, buffer ) {
    "use asm";

    var exp = stdlib.Math.exp;
    var log = stdlib.Math.log;
    var values = new stdlib.Float64Array(buffer);

    function logSum(start, end) {
        start = start|0;
        end = end|0;

        var sum = 0.0, p = 0, q = 0;

        // asm.js forces byte addressing of the heap by requiring shifting by 3
        for (p = start << 3, q = end << 3; (p|0) < (q|0); p = (p + 8)|0) {
            sum = sum + +log(values[p>>3]);
        }

        return +sum;
    }

    function geometricMean( start, end ) {
        start = start|0;
        end = end|0;
        return +exp(+logSum(start, end) / +((end - start)|0));
    }

    return { geometricMean: geometricMean };
} );

function init( start, val ) {
    var values = new Float64Array( am.heap );
    for( var i = start; i < values.length; i++ ) {
        values[ i ] = val;
    }
}

function test() {
    var v0 =  Date.now();
    var r0 = am.functions.geometricMean( 0, am.heap_size() / 8 );
    var v1 =  Date.now();

    console.log( Date.now() - v0 );
    console.log( r0 );
}

init( 0, 2 );
test();

os = am.heap_size();
am.resize_heap( os * 2 );
init( os / 8, 1 );
test();

var values = new Float64Array( am.heap );
console.log( values[ 0 ] );
console.log( values[ values.length - 1 ] );

// SIZE = 2*0x10000000, START = 0, END = SIZE / 8;
//var heap = new ArrayBuffer( 2 * SIZE );             // 64k heap
//END *= 2;
//init( heap, START, END, 20 );                   // fill a region with input values
//// fast = GeometricMean( typeof window === 'undefined' ? global : window, null, heap ); // produce exports object linked to AOT-compiled code
//v0 =  Date.now();
//var r1 = fast.geometricMean( START, END );      // computes geometric mean of input values
//console.log( Date.now() - v0 );
//console.log( r0 );
//console.log( r1 );


//var heap = new ArrayBuffer( 0x10000 );   // 64k heap
//var fast = PI8( global, null, heap );    // produce exports object linked to AOT-compiled code
//var resu = fast.pouet( 20, 30 ); // computes geometric mean of input values
//console.log( resu );

//try {
//    report = asm.validate( String( PI8 ) );
//    console.log( 'Validated!' );
//} catch( e ) {
//    console.error( e.stack );
//    console.error( e.src );
//    console.error( e.loc );
//}
