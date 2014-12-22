/// asm modules
/// manages the asm function + associated heap
/// (if the heap is resized, functions are re-compiled)
asm_mod = {
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

var __hasProp = {}.hasOwnProperty;
var __extends = function( child, parent ) {
    for( var key in parent ) {
        if ( __hasProp.call( parent, key ) )
            child[ key ] = parent[ key ];
    } 
    function ctor() { 
        this.constructor = child;
    }
    ctor.prototype = parent.prototype;
    child.prototype = new ctor();
    child.__super__ = parent.prototype;
    return child;
};
