//var socadb = require('./PI8.js');

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
        return res;
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

// PI8
var am_PI8 = am.push( function( stdlib, foreign, buffer ) {
    "use asm";

    var values = new stdlib.Uint8Array( buffer );

    function size_in_bits( start ) {
        start = start|0;
        return 8;
    }

    function get_val( start ) {
        start = start | 0;
        return values[ start ] | 0;
    }

    function set_val( start, val ) {
        start = start | 0;
        val   = val   | 0;
        values[ start ] = val;
    }

    return {
        size_in_bits: size_in_bits,
        get_val     : get_val,
        set_val     : set_val,
    };
} );

function PI8( offset ) {
    this.offset = offset == null ? am.allocate( 1 ) : offset;
}

Object.defineProperty( PI8.prototype, "size_in_bits", {
    get: function() {
        return am_PI8.size_in_bits( this.offset );
    }
});

Object.defineProperty( PI8.prototype, "val", {
    get: function() {
        return am_PI8.get_val( this.offset );
    },
    set: function( val ) {
        return am_PI8.set_val( this.offset, val );
    }
} );

// CUnsigned
var am_CUnsigned = am.push( function( stdlib, foreign, buffer ) {
    "use asm";

    var values = new stdlib.Uint8Array( buffer );

    function size_in_bits( start ) {
        start = start | 0;
        var n = 0;
        while ( ( values[ ( start + n ) | 0 ] | 0 ) >= 128 ) {
            n = ( n + 1 ) | 0;
        }
        return ( n + 1 ) | 0;
    }

    function get_val( start ) {
        start = start | 0;

        var res = 0;
        var v   = 0;

        v = values[ start ] | 0;
        if ( ( v | 0 ) < 128 )
            return v | 0;
        while ( 1 ) {
            start = ( start + 1 ) | 0;
            v = values[ start ] | 0;
            if ( ( v | 0 ) < 128 )
                return ( ( res * 128 ) | 0 + v ) | 0;
            res = ( ( res * 128 ) | 0 + ( v & 0x8F ) | 0 ) | 0;
        }
        return 0;
    }

    function set_val( start, val ) {
        start = start | 0;
        val   = val   | 0;

//        var current_val = 0;
//        while ( 1 ) {
//            if ( ( val | 0 ) < 128 ) {

//            }
//            v_old = ;
//        }

        values[ start ] = val;
    }

    function init( start ) {
        start = start | 0;
        values[ start ] = 0;
    }

    return {
        size_in_bits: size_in_bits,
        get_val     : get_val,
        set_val     : set_val,
        init        : init
    };
} );

function CUnsigned( offset ) {
    this.offset = offset == null ? am.allocate( 1 ) : offset;
    am_CUnsigned.init( this.offset );
}

Object.defineProperty( CUnsigned.prototype, "size_in_bits", {
    get: function() {
        return am_CUnsigned.size_in_bits( this.offset );
    }
});

Object.defineProperty( CUnsigned.prototype, "val", {
    get: function() {
        return am_CUnsigned.get_val( this.offset );
    },
    set: function( val ) {
        return am_CUnsigned.set_val( this.offset, val );
    }
} );

var a = new CUnsigned( 0 );
console.log( a.size_in_bits );
console.log( a.val );
a.val = 17;
console.log( a.val );

//// CString
//var am_CString = am.push( function( stdlib, foreign, buffer ) {
//    "use asm";

//    var values = new stdlib.Uint8Array( buffer );

//    function size_in_bits( start ) {
//        start = start|0;
//        var n = 0;
//        while (  )
//        return 8;
//    }

//    function get_val( start ) {
//        start = start | 0;
//        return values[ start ] | 0;
//    }

//    function set_val( start, val ) {
//        start = start | 0;
//        val   = val   | 0;
//        values[ start ] = val;
//    }

//    return {
//        size_in_bits: size_in_bits,
//        get_val     : get_val,
//        set_val     : set_val,
//    };
//} );

//function CString( offset ) {
//    this.offset = offset == null ? am.allocate( 1 ) : offset;
//}

//Object.defineProperty( CString.prototype, "size_in_bits", {
//    get: function() {
//        return am_CString.size_in_bits( this.offset );
//    }
//});

//Object.defineProperty( CString.prototype, "val", {
//    get: function() {
//        return am_CString.get_val( this.offset );
//    },
//    set: function( val ) {
//        return am_CString.set_val( this.offset, val );
//    }
//} );

//var v0 = new PI8( 0 );
//var v1 = new PI8( 1 );
//v0.val = 10
//console.log( v0.size_in_bits );
//console.log( v0.val );
//v0.val = 20;
//console.log( v0.val );
//console.log( v1.val );

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

//if ( typeof window === 'undefined' ) {
//    var asm = require('./asm.js/lib/asm.js');
//    try {
//        var report = asm.validate( String( PI8 ) );
//        // console.log( report );
//    } catch( e ) {
//        console.error( e.stack );
//        console.error( e.src );
//        console.error( e.loc );
//    }
//}
