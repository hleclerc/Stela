#include "../Ir/CallableFlags.h"
#include "../Ir/AssignFlags.h"
#include "../Ir/Numbers.h"
#include "IrWriter.h"
#include "Ast.h"

IrWriter::IrWriter( Ast *root ) {
    parse( root );
    int_reduction();
}

void IrWriter::parse( Ast *root ) {
    root->write_to( this );
}

ST IrWriter::size_of_binary_data() {
    ST res = 2;
    for( auto iter : nstrings )
        res += BinStreamWriter::size_needed_for( iter.first.size() ) + iter.first.size();
    return res + 1 + data.size();
}

void IrWriter::copy_binary_data_to( PI8 *ptr ) {
    // list of size + strings (not 0 ended)
    Vec<std::string> strings( Size(), nstrings.size() );
    for( auto iter : nstrings )
        strings[ iter.second ] = iter.first;

    for( int i = 0; i < strings.size(); ++i ) {
        // size
        ST s = strings[ i ].size();
        BinStreamWriter::write_val_to( ptr, s );
        // data
        memcpy( ptr, strings[ i ].c_str(), s );
        ptr += s;
    }
    *(ptr++) = 0;

    // tok data
    data.copy_to( ptr );
}

void IrWriter::push_delayed_parse( const Ast *l ) {
    int_to_reduce << IntToReduce{ data.size(), IntToReduce::OFFSET };

    if ( l ) {
        DelayedParse *dp = delayed_parse.push_back();
        dp->old_size = data.size();
        dp->offset = data.room_for();
        dp->l = l;

        *dp->offset = 0;
    } else {
        OffsetType *o = data.room_for();
        *o = 0;
    }
}

void IrWriter::push_potential_catched_vars_from( const Ast *l, std::set<String> avail ) {
    std::set<String> pot_needed;
    l->get_potentially_needed_ext_vars( pot_needed, avail );

    data << pot_needed.size();
    for( String s : pot_needed )
        push_nstring( s );
}

void IrWriter::int_reduction() {
    if ( int_to_reduce.size() == 0 )
        return;

    // copy of data
    Vec<PI8> dcp( Size(), data.size() );
    data.copy_to( dcp.ptr() );

    // get needed size for each int to reduce
    Vec<int> needed_size( Size(), int_to_reduce.size(), 8 );

    while ( true ) {
        // since offset are >= 0, one iteration should be enough
        bool changed = false;
        for( ST i = int_to_reduce.size() - 1; i >= 0; --i ) {
            const IntToReduce &itr = int_to_reduce[ i ];
            if ( itr.type == IntToReduce::OFFSET ) {
                // get "real" offset
                while ( true ) {
                    ST off = *reinterpret_cast<const PI64 *>( dcp.ptr() + itr.pos ), lim = itr.pos + off;
                    for( int j = i; j < int_to_reduce.size() and int_to_reduce[ j ].pos < lim; ++j )
                        off -= 8 - needed_size[ j ];
                    int ns = BinStreamWriter::size_needed_for( off );
                    if ( needed_size[ i ] != ns ) {
                        needed_size[ i ] = ns;
                        changed = true;
                    } else
                        break;
                }
            } else
                TODO;
        }
        if ( not changed )
            break;
    }

    //
    data.clear();
    data.write( dcp.ptr(), int_to_reduce[ 0 ].pos );
    for( ST i = 0; i < int_to_reduce.size(); ++i ) {
        const IntToReduce &itr = int_to_reduce[ i ];
        if ( itr.type == IntToReduce::OFFSET ) {
            SI64 off = *reinterpret_cast<const SI64 *>( dcp.ptr() + itr.pos ), lim = itr.pos + off;
            for( int j = i; j < int_to_reduce.size() and int_to_reduce[ j ].pos < lim; ++j )
                off -= 8 - needed_size[ j ];

            data << off;
            int a = itr.pos + 8;
            int b = i + 1 < int_to_reduce.size() ? int_to_reduce[ i + 1 ].pos : dcp.size();
            data.write( dcp.ptr() + a, b - a );
        }
    }
}


int IrWriter::nstring( const String &str ) {
    auto iter = nstrings.find( str );
    if ( iter != nstrings.end() )
        return iter->second;
    int res = nstrings.size();
    nstrings[ str ] = res;
    return res;
}

void IrWriter::push_nstring( const String &str ) {
    data << nstring( str );
}
