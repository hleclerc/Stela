#include "../Interpreter/Interpreter.h"
#include "../Inst/BaseType.h"
#include "../Inst/Inst.h"
#include "CppCompiler.h"
#include "CppInst.h"
#include <fstream>

enum {
    PREC_mul = 8,
    PREC_add = 10,
    PREC_phi = 20
};

PI64 CppInst::cur_op_id = 0;

CppInst::CppInst( int inst_id, int nb_outputs ) : inst_id( inst_id ), out( Size(), nb_outputs ), op_id( 0 ), op_id_vis( 0 ) {
}

void CppInst::DeclWriter::write_to_stream( Stream &os ) const {
    const BaseType *bt = info->out[ nout ].bt_hint;
    ASSERT( info->out[ nout ].num < 0, "weird" );
    ASSERT( bt, "bad" );

    info->out[ nout ].num = cc->get_free_reg( bt );
    os << *bt << " R" << info->out[ nout ].num << " = ";
}

void CppInst::InstWriter::write_to_stream( Stream &os ) const {
    if ( info->out[ nout ].num >= 0 )
        os << "R" << info->out[ nout ].num;
    else
        info->write_code( cc, precedance );
}

CppInst::DeclWriter CppInst::decl( CppCompiler *cc, int nout ) {
    DeclWriter res;
    res.info = this;
    res.nout = nout;
    res.cc = cc;
    return res;
}

CppInst::InstWriter CppInst::inst( CppCompiler *cc, int nout, int precedance ) {
    InstWriter res;
    res.precedance = precedance;
    res.info = this;
    res.nout = nout;
    res.cc = cc;
    return res;
}

void CppInst::add_child( CppExpr expr ) {
    expr.inst->out[ expr.nout ].parents << Out::Parent{ this, int( inp.size() ) };
    inp << expr;
}

const BaseType *CppInst::inp_bt_hint( int ninp ) const {
    switch ( inst_id ) {
        #define DECL_IR_TOK( INST ) case Inst::Id_Op_##INST:
        #include "../Ir/Decl_Operations.h"
        #undef DECL_IR_TOK
            return *reinterpret_cast<const BaseType **>( additionnal_data );
    case Inst::Id_Conv:
        return reinterpret_cast<const BaseType **>( additionnal_data )[ 1 ];
    case Inst::Id_Syscall:
        return ninp ? ip->bt_ST : bt_Void;
    case Inst::Id_Phi:
        for( const CppInst::Out::Parent &p : out[ 0 ].parents )
            if ( const BaseType *res = p.inst->inp_bt_hint( p.ninp ) )
                return res;
    }
    return 0;
}

const BaseType *CppInst::out_bt_hint( int nout ) const {
    // already done ?
    if ( const BaseType *res = out[ nout ].bt_hint )
        return res;

    // instruction dependant
    switch ( inst_id ) {
    #define DECL_IR_TOK( INST ) case Inst::Id_Op_##INST:
    #include "../Ir/Decl_Operations.h"
    #undef DECL_IR_TOK
        return *reinterpret_cast<const BaseType **>( additionnal_data );
    case Inst::Id_Conv:
         return reinterpret_cast<const BaseType **>( additionnal_data )[ 0 ];
    default:
        return 0;
    }
}

const BaseType *CppInst::get_bt_hint_for_nout( int nout ) const {
    // local hint ?
    if ( const BaseType *res = out_bt_hint( nout ) )
         return res;
    // else, look in parent inputs
    for( const CppInst::Out::Parent &p : out[ nout ].parents )
        if ( const BaseType *res = p.inst->inp_bt_hint( p.ninp ) )
            return res;
    return 0;
}

void CppInst::update_bt_hints() const {
    for( int i = 0; i < out.size(); ++i )
        if ( not out[ i ].bt_hint )
            out[ i ].bt_hint = get_bt_hint_for_nout( i );
}

void CppInst::write_to_stream( Stream &os ) const {
    update_bt_hints();

    // particular cases
    if ( inst_id == Inst::Id_Cst ) {
        if ( out[ 0 ].bt_hint )
            return out[ 0 ].bt_hint->write_to_stream( os, additionnal_data + sizeof( int ) );

        // else
        int size = *reinterpret_cast<int *>( additionnal_data );

        const PI8 *data = additionnal_data + sizeof( int );
        const char *c = "0123456789ABCDEF";
        for( int i = 0; i < std::min( size / 8, 4 ); ++i ) {
            if ( i )
                os << ' ';
            os << c[ data[ i ] >> 4 ] << c[ data[ i ] & 0xF ];
        }
        if ( size / 8 > 4 )
            os << "...";
        return;
    }

    switch ( inst_id ) {
        #define DECL_INST( INST ) case Inst::Id_##INST: os << #INST; break;
        #include "../Inst/DeclInst.h"
        #undef DECL_INST
        default: ERROR( "?" );
    }
}

void CppInst::write_code_bin_op( CppCompiler *cc, int prec, const char *op_str, int prec_op ) {
    if ( prec >= 0 or out[ 0 ].parents.size() > 1 ) {
        if ( prec < 0 )
            cc->on.write_beg() << decl( cc, 0 );
        cc->os << inp[ 0 ].inst->inst( cc, inp[ 0 ].nout, prec_op ) << op_str << inp[ 1 ].inst->inst( cc, inp[ 1 ].nout, prec_op );
        if ( prec < 0 )
            cc->on.write_end( ";" );
    }
}

void CppInst::write_code( CppCompiler *cc, int prec ) {
    switch ( inst_id ) {
    case Inst::Id_Cst:
        if ( prec >= 0 )
            out[ 0 ].bt_hint->write_to_stream( cc->os, additionnal_data + sizeof( int ) );
        break;
    case Inst::Id_Rand:
        cc->on << decl( cc, 0 ) << "rand();";
        break;
    case Inst::Id_Conv:
        if ( prec >= 0 )
            cc->os << *out[ 0 ].bt_hint << "( " << inp[ 0 ].inst->inst( cc, inp[ 0 ].nout ) << " )";
        else if ( out[ 0 ].parents.size() > 1 )
            cc->on << decl( cc, 0 ) << inp[ 0 ].inst->inst( cc, inp[ 0 ].nout ) << ";";
        break;
    case Inst::Id_Phi:
        if ( prec >= 0 or out[ 0 ].parents.size() > 1 ) {
            if ( prec < 0 )
                cc->on.write_beg() << decl( cc, 0 );
            cc->os << inp[ 0 ].inst->inst( cc, inp[ 0 ].nout, PREC_phi ) << " ? "
                   << inp[ 1 ].inst->inst( cc, inp[ 1 ].nout, PREC_phi ) << " : "
                   << inp[ 2 ].inst->inst( cc, inp[ 2 ].nout, PREC_phi );
            if ( prec < 0 )
                cc->on.write_end( ";" );
        }
        break;
    case Inst::Id_Syscall:
        cc->on.write_beg();
        if ( out[ 1 ].parents.size() )
            cc->os << decl( cc, 1 );
        cc->os << "syscall( ";
        for( int i = 1; i < inp.size(); ++i ) {
            if ( i > 1 )
                cc->os << ", ";
            cc->os << inp[ i ].inst->inst( cc, inp[ i ].nout, 0 );
        }
        cc->on.write_end( " );" );
        break;

    case Inst::Id_Op_add: write_code_bin_op( cc, prec, " + ", PREC_add ); break;
    case Inst::Id_Op_mul: write_code_bin_op( cc, prec, " * ", PREC_mul ); break;

    default:
        PRINT( inst_id );
        // TODO;
    }
}

int CppInst::display_graph( const Vec<CppInst *> &res, const char *filename ) {
    ++cur_op_id;

    std::ofstream f( filename );
    f << "digraph Instruction {";
    f << "  node [shape = record];";
    for( int i = 0; i < res.size(); ++i )
        res[ i ]->write_graph_rec( f );
    f << "}";
    f.close();

    return system( ( "dot -Tps " + std::string( filename ) + " > " + std::string( filename ) + ".eps && gv " + std::string( filename ) + ".eps" ).c_str() );
}

void CppInst::write_graph_rec( Stream &os ) const {
    if ( op_id_vis == cur_op_id )
        return;
    op_id_vis = cur_op_id;

    // label
    std::ostringstream ss;
    write_to_stream( ss );

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
    for( int i = 0; i < out.size(); ++i ) {
        os << "|<f" << i << ">";
        if ( out[ i ].bt_hint )
            os << *out[ i ].bt_hint;
        else
            os << "o";
    }
    for( int i = 0; i < inp.size(); ++i )
        os << "|<f" << out.size() + i << ">i";
    os << "\"];\n";

    // children
    for( int i = 0, n = inp.size(); i < n; ++i ) {
        const CppExpr &ch = inp[ i ];
        os << "    node" << this << ":f" << out.size() + i << " -> node" << ch.inst << ":f" << ch.nout << ";\n";
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

