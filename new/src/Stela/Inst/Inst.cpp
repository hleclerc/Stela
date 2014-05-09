#include <fstream>
#include "Inst.h"
#include "Ip.h"

PI64 Inst::cur_op_id = 0;

Inst::~Inst() {
    ext_par   = 0;

    op_id_vis = 0;
    op_id     = 0;
    op_mp     = 0;
}

void Inst::write_to_stream( Stream &os ) const {
    write_dot( os );
    if ( inp.size() ) {
        for( int i = 0; i < inp.size(); ++i )
            os << ( i ? "," : "(" ) << inp[ i ];
        os << ")";
    }
}

void Inst::add_var_ptr( Var *var ) {
    write_dot( std::cerr );
    IP_ERROR( "not an Inst based on a Room instance" );
}

void Inst::set( Ptr<Inst> val ) {
    write_dot( std::cerr );
    IP_ERROR( "not an Inst based on a Room instance" );
}

void Inst::add_inp( Ptr<Inst> val ) {
    val->par << Parent{ this, int( inp.size() ) };
    inp << val;
}

int Inst::display_graph( const Vec<Ptr<Inst> > &outputs, const char *filename ) {
    ++cur_op_id;

    std::ofstream f( filename );
    f << "digraph Instruction {\n";
    f << "  node [shape = record];\n";

    Vec<const Inst *> ext_buf;
    for( int i = 0; i < outputs.size(); ++i )
        outputs[ i ]->write_graph_rec( ext_buf, f );

    for( const Inst *ch : ext_buf )
        if ( ch )
            ch->write_sub_graph_rec( f );

    f << "}";
    f.close();

    return system( ( "dot -Tps " + std::string( filename ) + " > " + std::string( filename ) + ".eps && gv " + std::string( filename ) + ".eps" ).c_str() );
}

void Inst::write_sub_graph_rec( Stream &os ) const {
    os << "    node" << ext_par << " -> node" << this << " [color=\"green\"];\n";
    os << "    subgraph cluster_" << this <<" {\ncolor=yellow;\nstyle=dotted;\n";
    Vec<const Inst *> ext_buf;
    write_graph_rec( ext_buf, os );
    for( const Inst *nch : ext_buf )
        nch->write_sub_graph_rec( os );
    os << "    }\n";
}

void Inst::write_graph_rec( Vec<const Inst *> &ext_buf, Stream &os ) const {
    if ( op_id_vis == cur_op_id )
        return;
    op_id_vis = cur_op_id;

    // label
    std::ostringstream ss;
    write_dot( ss );

    // node
    std::string ls = ss.str();
    os << "    node" << this << " [label=\"";
    for( unsigned i = 0; i < ls.size(); ++i ) {
        switch ( ls[ i ] ) {
        case '<':
        case '>':
        case '\\':
            os << '\\';
        }
        os << ls[ i ];
    }
    if ( inp.size() > 1 )
        for( int i = 0; i < inp.size(); ++i )
            os << "|<f" << i << ">i";
    os << "\"];\n";

    // children
    for( int i = 0; i < inp.size(); ++i ) {
        os << "    node" << this;
        if ( inp.size() > 1 )
            os << ":f" << i;
        os << " -> node" << inp[ i ].ptr() << ";\n";

        if ( inp[ i ] )
            inp[ i ]->write_graph_rec( ext_buf, os );
    }

    // ext
    for( int i = 0; i < ext.size(); ++i )
        if ( const Inst *ch = ext[ i ].ptr() )
            ext_buf << ch;
}

Ptr<Inst> Inst::_simplified() {
    return 0;
}

Ptr<Inst> Inst::_pointer_on( int beg, int len ) {
    return 0;
}

Ptr<Inst> simplified( Ptr<Inst> val ) {
    if ( Ptr<Inst> res = val->_simplified() )
        return res;
    return val;
}
