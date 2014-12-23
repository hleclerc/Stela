# asm modules
# manages the asm function + associated heap
# (if the heap is resized, functions are re-compiled)
class AsmMod
    @page_size: 8192
    @global   : if window? then window else global
    @sizes    : [ 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 ]

    constructor: ->
        @modules = [] # asm.js modules (where the code is defined)
        @heap    = new ArrayBuffer 0x10000 # must be a multiple of AsmMod.page_size
        
        # map size -> free: [{ offset: ..., ptr_last_free_elem: ..., occupation_ratio: ... }}, full: [ offsets ]
        @free_ptrs = ( 1 for i in [ 0 ... 11 ] )
        
        # ordered list of free pages
        @free_pages = ( i for i in [ 0 ... @get_heap_size() ] by AsmMod.page_size )
        
        # page number -> list of references to @free_ptr_for_size_... for each page
        @pages_info = for i in [ 0 ... @get_heap_size() ] by AsmMod.page_size
            occupation: 0
            size      : 0
            
        
    push: ( module ) ->
        @modules.push module
        module AsmMod.global, null, @heap
    
    get_heap_size: ->
        @heap.byteLength
    
    get_occupation: ->
        nb_pages = ( @get_heap_size() / AsmMod.page_size ) | 0
        res = ( nb_pages - @free_pages.length ) * AsmMod.page_size
        
    resize_heap: ( new_size ) ->
        # a new heap
        old_heap = @heap
        old_size = @heap.byteLength
        @heap = new ArrayBuffer new_size
        new Uint8Array( @heap, 0, old_heap.byteLength ).set new Uint8Array old_heap
        
        # re-compilation of all the modules
        for module in @modules
            module AsmMod.global, null, @heap

        # update @free_pages
        while old_size < new_size
            @free_pages.push old_size
            
            @pages_info.push
                occupation: 0
                size      : 0
                
            old_size += AsmMod.page_size

    allocate: ( size ) ->
        # big size ?
        if size >= AsmMod.page_size
            return @_allocate_pages ( ( size + AsmMod.page_size - 1 ) / AsmMod.page_size ) | 0
        # else, find size (look in @busy_pages)
        num_list = @_num_list_for_size size
        ptr = @free_ptrs[ num_list ]
        if ptr == 1
            ptr = @_allocate_chuncks size
        @free_ptrs[ num_list ] = ( new Uint32Array @heap, ptr, 4 )[ 0 ]

        pi = @pages_info[ ( ptr / AsmMod.page_size ) | 0 ]
        pi.occupation++
        
        return {
            ptr : ptr
            rese: AsmMod.sizes[ num_list ]
        }
    
    free: ( rese, ptr ) ->
        # big size ?
        if size >= AsmMod.page_size
            num_page = ( ptr / AsmMod.page_size ) | 0
            nb_pages = ( ( size + AsmMod.page_size - 1 ) / AsmMod.page_size ) | 0
            return @_desallocate_pages num_page, nb_pages
        # else, find size (look in @busy_pages)
        num_list = @_num_list_for_size size
        todo()

    _num_list_for_size: ( s ) ->
        # TODO: choose the if sequence according to the actual needs in terms of size (i.e. avoid a basic red-black tree)
        if s <= 32
            if s <= 8
                if s <= 4
                    return 0
                return 1
            if s <= 16
                return 2
            return 3
        if s <= 256
            if s <= 128
                if s <= 64
                    return 4
                return 5
            return 6
        if s <= 1024
            if s <= 512
                return 7
            return 8
        if s <= 2048
            return 9
        return 10
        
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
        
    _desallocate_pages: ( num_page, nb_pages ) ->
        todo()
        
    _allocate_chuncks: ( size ) ->
        res = @_allocate_pages 1
        a32 = new Uint32Array @heap, res, AsmMod.page_size
        for i in [ 0 ... AsmMod.page_size - 1 ] by size
            a32[ i / 4 ] = res + i + size
        a32[ ( AsmMod.page_size - size ) / 4 ] = 1 # an impossible adress
        
        pi = @pages_info[ ( res / AsmMod.page_size ) | 0 ]
        pi.size = size
        
        res

asm_mod = new AsmMod
