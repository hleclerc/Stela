class PI8 extends Model
    am = asm_mod.push ( stdlib, foreign, buffer ) ->
        "use asm"

        values = new stdlib.Uint8Array buffer

        size_in_bits = ( start ) ->
            start = start | 0
            return 8

        get_val = ( start ) ->
            start = start | 0
            values[ start ] | 0

        set_val = ( start, val ) ->
            start = start | 0
            val   = val   | 0
            values[ start ] = val

        return {
            size_in_bits: size_in_bits
            get_val     : get_val
            set_val     : set_val
        }

    constructor: ( offset ) ->
        super offset

    Object.defineProperty PI8.prototype, "size_in_bits",
        get: () -> am.size_in_bits @offset

    Object.defineProperty PI8.prototype, "val",
        get: () -> am.get_val @offset
        set: ( val ) ->
            @__sig_change?()
            am.set_val @offset, val
