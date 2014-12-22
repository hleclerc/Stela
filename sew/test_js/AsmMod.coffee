# asm modules
# manages the asm function + associated heap
# (if the heap is resized, functions are re-compiled)
asm_mod =
    modules    : []
    functions  : {}
    heap       : new ArrayBuffer 0x10000000
    global     : if window? then window else global
    heap_size  : () -> @heap.byteLength
    push       : ( module ) ->
        @modules.push module
        res = module @global, null, @heap
        for v in res
            @functions[ v ] = res[ v ]
        res
    resize_heap: ( new_size ) ->
        # a new heap
        old_heap = @heap
        @heap = new ArrayBuffer new_size
        new Uint8Array( @heap, 0, old_heap.byteLength ).set new Uint8Array old_heap
        
        # re-compilation of all the modules
        for module in @modules
            res = module @global, null, @heap
            for v of res
                @functions[ v ] = res[ v ]

# __hasProp = {}.hasOwnProperty;
# __extends = function( child, parent ) {
#     for( var key in parent ) {
#         if ( __hasProp.call( parent, key ) )
#             child[ key ] = parent[ key ];
#     } 
#     function ctor() { 
#         @constructor = child;
#     }
#     ctor.prototype = parent.prototype;
#     child.prototype = new ctor();
#     child.__super__ = parent.prototype;
#     return child;
# };
