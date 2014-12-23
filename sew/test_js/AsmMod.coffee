# asm modules
# manages the asm function + associated heap
# (if the heap is resized, functions are re-compiled)
class AsmMod
    @page_size: 8192
    @global   : if window? then window else global
    
    constructor: ->
        @modules = [] # asm.js modules (where the code is defined)
        @heap    = new ArrayBuffer 0x10000 # must be a multiple of AsmMod.page_size
        
        # map size -> free: [{ offset: ..., ptr_last_free_elem: ..., occupation_ratio: ... }}, full: [ offsets ]
        @pages_for_size_4    = { free: [], full: [] }
        @pages_for_size_8    = { free: [], full: [] }
        @pages_for_size_16   = { free: [], full: [] }
        @pages_for_size_32   = { free: [], full: [] }
        @pages_for_size_64   = { free: [], full: [] }
        @pages_for_size_128  = { free: [], full: [] }
        @pages_for_size_256  = { free: [], full: [] }
        @pages_for_size_512  = { free: [], full: [] }
        @pages_for_size_1024 = { free: [], full: [] }
        @pages_for_size_2048 = { free: [], full: [] }
        @pages_for_size_4096 = { free: [], full: [] }
                
        # ordered list of free pages (without any chuncks on it)
        @free_pages = i for i in [ 0 ... @get_heap_size() ] by AsmMod.page_size
        
        # list of references to @pages_for_size_... for each page
        @pages_list = { undefined } for i in [ 0 ... @get_heap_size() ] by AsmMod.page_size
        
    push: ( module ) ->
        @modules.push module
        module AsmMod.global, null, @heap
    
    get_heap_size: ->
        @heap.byteLength
        
    resize_heap: ( new_size ) ->
        # a new heap
        old_heap = @heap
        old_size = @heap.byteLength
        @heap = new ArrayBuffer new_size
        console.log "new size", new_size
        new Uint8Array( @heap, 0, old_heap.byteLength ).set new Uint8Array old_heap
        
        # re-compilation of all the modules
        for module in @modules
            module AsmMod.global, null, @heap

        # update @free_pages
        while old_size < new_size
            @free_pages.push old_size
            old_size += AsmMod.page_size

    allocate: ( size ) ->
        # big size ?
        if size >= AsmMod.page_size
            return @_allocate_pages( ( size / AsmMod.page_size ) | 0 )
        # else, find size (look in @busy_pages)
        next_power_of_two = ( v ) ->
            --v
            v |= v >> 1
            v |= v >> 2
            v |= v >> 4
            v |= v >> 8
            v |= v >> 16
            ++v
        size = if size <= 4 then 4 else next_power_of_two size
        # 
        p = @busy_pages.get size
        if not p.free.length
            o = @_allocate_chuncks size
            p.free.push
                offset            : o
                ptr_last_free_elem: o
                occupation        : 0
        num_free_list = 0
        f = p.free[ num_free_list ]
        res = f.ptr_last_free_elem
        f.ptr_last_free_elem = ( new Uint32Array @heap, res, 4 )[ 0 ]
        f.occupation++
        if f.ptr_last_free_elem == 1
            p.full.push f.offset
            p.free.splice num_free_list, 1
        return {
            ptr : res
            rese: size
        }
    
    free: ( rese, ptr ) ->
        p = @busy_pages.get rese

    _pages_for_size: ( s ) ->
        # TODO: choose the if sequence according to the actual needs in terms of size (i.e. avoid a basic red-black tree)
        if s <= 32
            if s <= 8
                if s <= 4
                    return @pages_for_size_4
                return @pages_for_size_8
            if s <= 16
                return @pages_for_size_16
            return @pages_for_size_32
        if s <= 256
            if s <= 128
                if s <= 64
                    return @pages_for_size_64
                return @pages_for_size_128
            return @pages_for_size_256
        if s <= 1024
            if s <= 512
                return @pages_for_size_512
            return @pages_for_size_1024
        if s <= 2048
            return @pages_for_size_2048
        return @pages_for_size_4096
        
    _allocate_pages: ( nb_pages ) ->
        while true
            # helper
            contiguous = ( i ) ->
                for o in [ 1 ... nb_pages ]
                    if @free_pages[ i + o ] != @free_pages[ i + o - 1 ] + AsmMod.page_size
                        return false
                true
            # find a set of contiguous free pages
            for i in [ 0 ... @free_pages.length ]
                if contiguous i
                    res = @free_pages[ i ]
                    @free_pages.splice i, nb_pages
                    return res
            # if not found, resize the heap and retry
            @resize_heap @get_heap_size() * 2
        
    _allocate_chuncks: ( size ) ->
        res = @_allocate_pages 1
        a32 = new Uint32Array @heap, res, AsmMod.page_size
        for i in [ 0 ... AsmMod.page_size - 1 ] by size
            a32[ i / 4 ] = res + i + size
        a32[ ( AsmMod.page_size - size ) / 4 ] = 1 # an impossible adress
        res
        
    
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

asm_mod = new AsmMod
