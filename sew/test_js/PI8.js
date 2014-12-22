PI8 = (function() {
    __extends( PI8, Model );
    
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

    // constructor
    function PI8( offset ) {
        PI8.__super__.constructor.call( this, offset );
    }

    Object.defineProperty( PI8.prototype, "size_in_bits", {
        get: function() {
            return am.size_in_bits( this.offset );
        }
    });

    Object.defineProperty( PI8.prototype, "val", {
        get: function() {
            return am.get_val( this.offset );
        },
        set: function( val ) {
            if ( this.__sig_change != null )
                this.__sig_change();
            return am.set_val( this.offset, val );
        }
    } );
    
    return PI8;
})();
