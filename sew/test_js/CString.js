CString = (function() {
    var am = asm_mod.push( function( stdlib, foreign, buffer ) {
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

    function CString( offset ) {
        this.offset = offset == null ? am.allocate( 1 ) : offset;
    }

    Object.defineProperty( CString.prototype, "size_in_bits", {
        get: function() {
            return am.size_in_bits( this.offset );
        }
    });

    Object.defineProperty( CString.prototype, "val", {
        get: function() {
            return am.get_val( this.offset );
        },
        set: function( val ) {
            return am.set_val( this.offset, val );
        }
    } );

    Object.defineProperty( Model.prototype, "attr_names", {
        get: function() { return ["size","data"]; }
    }
    
    return CString;
})();
