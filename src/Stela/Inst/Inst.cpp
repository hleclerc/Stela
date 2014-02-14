#include <fstream>
#include "Cst.h"

PI64 Inst::cur_op_id = 0;

Inst::Inst() {
    ext_parent = 0;

    op_id_vis  = 0;
    op_id      = 0;
    op_mp      = 0;
}

Inst::~Inst() {
}

int Inst::size_in_bytes( int nout ) const {
    return ( size_in_bits( nout ) + 7 ) / 8;
}

void Inst::write_to_stream( Stream &os ) const {
    write_dot( os );
    os << "(";
    for( int i = 0; i < inp_size(); ++i ) {
        if ( i )
            os << ",";
        os << inp_expr( i );
    }
    os << ")";
}

const PI8 *Inst::cst_data( int nout, int beg, int end ) const {
    return 0;
}

const PI8 *Inst::vat_data( int nout, int beg, int end ) const {
    return 0;
}

const BaseType *Inst::out_bt( int n ) const {
    return 0;
}

bool Inst::equal( const Inst *b ) const {
    if ( inst_id() != b->inst_id() or inp_size() != b->inp_size() )
        return false;
    for( int i = 0; i < inp_size(); ++i )
        if ( inp_expr( i ) != b->inp_expr( i ) )
            return false;
    return true;
}

int Inst::display_graph( const Vec<ConstPtr<Inst> > &outputs, const char *filename ) {
    ++cur_op_id;

    std::ofstream f( filename );
    f << "digraph Instruction {";
    f << "  node [shape = record];";
    for( int i = 0; i < outputs.size(); ++i )
        outputs[ i ]->write_graph_rec( f );
    f << "}";
    f.close();

    return system( ( "dot -Tps " + std::string( filename ) + " > " + std::string( filename ) + ".eps && gv " + std::string( filename ) + ".eps" ).c_str() );
}

void Inst::write_graph_rec( Stream &os ) const {
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
    for( int i = 0; i < out_size(); ++i )
        os << "|<f" << i << ">o";
    for( int i = 0; i < inp_size(); ++i )
        os << "|<f" << out_size() + i << ">i";
    os << "\"];\n";

    // children
    for( int i = 0, n = inp_size(); i < n; ++i ) {
        const Expr &ch = inp_expr( i );
        os << "    node" << this << ":f" << out_size() + i << " -> node" << ch.inst.ptr() << ":f" << ch.nout << ";\n";
        if ( ch.inst )
            ch.inst->write_graph_rec( os );
    }

    // ext
    // for( int i = 0, n = ext_disp_size(); i < n; ++i ) {
    //        const Inst *ch = ext_disp_inst( i );
    //        os << "    node" << this << " -> node" << ch << " [color=\"green\"];\n";
    //        if ( ch ) {
    //            os << "    subgraph cluster_" << ch <<" {\ncolor=yellow;\nstyle=dotted;\n";
    //            ch->write_graph_rec( os, omd );
    //            os << "    }\n";
    //        }
    //    }

    // parents
    //    for( int nout = 0; nout < out_size(); ++nout ) {
    //        VPar &p = parents( nout );
    //        for( int i = 0; i < p.size(); ++i ) {
    //            os << "    node" << p[ i ] << " -> node" << this << ":f" << nout << " [color=\"red\"];\n";
    //            // parents[ i ]->write_graph_rec( os );
    //        }
    //    }
}


Inst *Inst::factorized( Inst *inst ) {
    if ( inst->inp_size() ) {
        const Vec<Inst::Out::Parent,-1,1> &p = inst->inp_expr( 0 ).parents();
        for( int i = 0; i < p.size(); ++i ) {
            if ( p[ i ].inst != inst and p[ i ].inst->equal( inst ) ) {
                delete inst;
                return p[ i ].inst;
            }
        }
    }
    return inst;
}

Expr Inst::_smp_slice( int nout, int beg, int end ) {
    if ( beg == 0 and end == size_in_bits( nout ) )
        return Expr( this, nout );
    if ( beg == end )
        return cst( 0, 0, 0 );
    return Expr();
}

Expr Inst::_smp_val_at( int nout, int beg, int end ) {
    return Expr();
}

Expr Inst::_smp_pointer_on( int nout ) {
    return Expr();
}
