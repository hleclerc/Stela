CUnsigned = (function() {
    var am = asm_mod.push( function( stdlib, foreign, buffer ) {
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

    function CUnsigned( offset, parent ) {
        this.offset = offset == null ? am.allocate( 1 ) : offset;
        this.parent = parent == null ? undefined : parent;
        am.init( this.offset );
    }

    Object.defineProperty( CUnsigned.prototype, "size_in_bits", {
        get: function() {
            return am.size_in_bits( this.offset );
        }
    });

    Object.defineProperty( CUnsigned.prototype, "val", {
        get: function() {
            return am.get_val( this.offset );
        },
        set: function( val ) {
            return am.set_val( this.offset, val );
        }
    } );
    
    return CUnsigned;
})();
