class PI8 extends Model
    `
    var am = asm_mod.push( function( stdlib, foreign, buffer ) {
        "use asm";

        var values = new stdlib.Uint8Array( buffer );

        function size_in_bits( start ) {
            start = start | 0;
            return 8;
        }

        function get_val( start ) {
            start = start | 0;
            return values[ start ] | 0;
        }

        function set_val( start, val ) {
            start = start | 0;
            val   = val   | 0;
            res   = ( values[ start ] | 0 ) != val;
            values[ start ] = val;
            return res | 0;
        }

        return {
            size_in_bits: size_in_bits,
            get_val     : get_val,
            set_val     : set_val
        };
    } );
    `

    constructor: ( ptr ) ->
        super ptr
        console.log "ptr:", @__ptr

    get_size_in_bits: ->
        am.size_in_bits @__ptr

    get_val: ->
        am.get_val @__ptr

    set_val: ( val ) ->
        am.set_val @__ptr, val
