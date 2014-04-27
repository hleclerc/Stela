#include "../Interpreter/Interpreter.h"
#include "../Inst/SetValData.h"
#include "../Inst/BaseType.h"
#include "../System/rcast.h"
#include "CppCompiler.h"
#include "CppInst.h"
#include <fstream>
#include <limits>

static bool full_dot = true;

enum {
    PREC_phi    =  1,

    PREC_equ    = 10,
    PREC_neq    = 10,
    PREC_sup    = 10,
    PREC_inf    = 10,
    PREC_sup_eq = 10,
    PREC_inf_eq = 10,

    PREC_or     = 15,
    PREC_and    = 15,

    PREC_not    = 17,

    PREC_add    = 20,
    PREC_sub    = 20,
    PREC_mul    = 30,
    PREC_div    = 30,
    PREC_mod    = 30,


    PREC_none
};

static bool same_op( int prec_op ) {
    switch ( prec_op ) {
    case PREC_add:
    // case PREC_sub:
    case PREC_mul:
    //case PREC_div:
    //case PREC_mod:
        return true;
    default:
        return false;
    }
}

PI64 CppInst::cur_op_id = 0;

CppInst::CppInst( int inst_id, int nb_outputs ) : inst_id( inst_id ), out( Size(), nb_outputs ), op_id( 0 ), op_id_vis( 0 ) {
    ext_parent = 0;
    ext_ds = -1;

    write_break = false;
    inlined = false;
}

void CppInst::DeclWriter::write_to_stream( Stream &os ) const {
    const BaseType *bt = info->out[ nout ].bt_hint;
    cc->bt_to_decl.insert( bt );
    if ( not bt ) {
        PRINT( *info );
        ERROR( "bad" );
    }

    if ( info->out[ nout ].num < 0 )
        info->out[ nout ].num = cc->get_free_reg( bt );

    if ( equ_sgn )
        os << "R" << info->out[ nout ].num << " = ";
}

void CppInst::InstWriter::write_to_stream( Stream &os ) const {
    if ( ptr ) {
        ASSERT( info->out[ nout ].num >= 0, "..." );
        if ( info->out[ nout ].bt_hint->size_in_bits() != 8 )
            os << "(char *)";
        os << "&R" << info->out[ nout ].num;
        return;
    }

    if ( info->inst_id == CppInst::Id_IfInp ) {
        InstWriter n = *this;
        n.info = info->ext_parent->inp[ nout ].inst;
        n.nout = info->ext_parent->inp[ nout ].nout;
        return n.write_to_stream( os );
    }

    if ( info->out[ nout ].num >= 0 )
        os << "R" << info->out[ nout ].num;
    else
        info->write_code( cc, precedance );
}

CppInst::DeclWriter CppInst::decl( CppCompiler *cc, int nout, bool equ_sgn ) {
    DeclWriter res;
    res.equ_sgn = equ_sgn;
    res.info = this;
    res.nout = nout;
    res.cc = cc;
    return res;
}

CppInst::InstWriter CppInst::inst( CppCompiler *cc, int nout, int precedance, bool ptr ) {
    InstWriter res;
    res.precedance = precedance;
    res.info = this;
    res.nout = nout;
    res.ptr = ptr;
    res.cc = cc;
    return res;
}

CppInst::InstWriter CppInst::disp( CppCompiler *cc, CppExpr expr, int precedance, bool ptr ) {
    return expr.inst->inst( cc, expr.nout, precedance, ptr );
}

int CppInst::add_child( CppExpr expr ) {
    int res = inp.size();
    if ( expr.inst )
        expr.inst->out[ expr.nout ].parents << Out::Parent{ this, int( inp.size() ) };
    inp << expr;
    return res;
}

void CppInst::set_child( int ninp, CppExpr expr ) {
    if ( inp[ ninp ].inst )
        inp[ ninp ].inst->out[ inp[ ninp ].nout ].parents.remove_first_unordered( Out::Parent{ this, ninp } );
    if ( expr.inst )
        expr.inst->out[ expr.nout ].parents << Out::Parent{ this, ninp };
    inp[ ninp ] = expr;
}

int CppInst::add_out() {
    int res = out.size();
    out.push_back();
    return res;
}

void CppInst::check_out_size( int n ) {
    if ( out.size() < n )
        out.resize( n );
}

void CppInst::add_ext( CppInst *inst ) {
    ASSERT( not inst->ext_parent, "..." );
    inst->ext_parent = this;
    ext << inst;
}

void CppInst::set_out_bt_hint( int nout, const BaseType *bt, bool force ) {
    if ( nout >= out.size() )
        return;
    if ( force ? out[ nout ].bt_hint == bt : out[ nout ].bt_hint != 0 )
        return;
    out[ nout ].bt_hint = bt;

    // parent propagation
    for( const CppInst::Out::Parent &p : out[ nout ].parents )
        p.inst->set_inp_bt_hint( p.ninp, bt );

    // downward propagation
    switch ( inst_id ) {
    case CppInst::Id_WhileInp: {
        CppExpr ch = ext_parent->inp[ nout ];
        ch.inst->set_out_bt_hint( ch.nout, bt );
        break;
    }
    case CppInst::Id_IfInp: {
        CppExpr ch = ext_parent->inp[ nout ];
        ch.inst->set_out_bt_hint( ch.nout, bt );
        break;
    }
    case CppInst::Id_WhileInst: {
        CppExpr ch = ext[ 0 ]->inp[ nout ];
        ch.inst->set_out_bt_hint( ch.nout, bt );
        break;
    }
    case CppInst::Id_If: {
        for( int next = 0; next < 2; ++next ) {
            CppExpr ch = ext[ next ]->inp[ nout ];
            ch.inst->set_out_bt_hint( ch.nout, bt );
        }
        break;
    }
    }
}

void CppInst::set_inp_bt_hint( int ninp, const BaseType *bt ) {
    switch ( inst_id ) {
    case CppInst::Id_WhileOut:
        if ( ninp < inp.size() - 1 )
            ext_parent->set_out_bt_hint( ninp, bt );
        break;
    case CppInst::Id_IfOut:
        ext_parent->set_out_bt_hint( ninp, bt );
        break;
    case CppInst::Id_WhileInst:
        ext[ 1 ]->set_out_bt_hint( ninp, bt );
        break;
    case CppInst::Id_If:
        for( int nch = 2; nch < 4; ++nch )
            ext[ nch ]->set_out_bt_hint( ninp, bt );
        break;
    case CppInst::Id_SetVal:
        if ( ninp == 0 )
            set_out_bt_hint( 0, bt );
        break;
    }
}

void CppInst::bt_hint_propagation() {
    switch ( inst_id ) {
    // case Op_...:
        #define DECL_IR_TOK( INST ) case CppInst::Id_Op_##INST:
        #include "../Ir/Decl_BinaryBoolOperations.h"
        #include "../Ir/Decl_UnaryBoolOperations.h"
        #undef DECL_IR_TOK
        set_out_bt_hint( 0, bt_Bool );
        for( int i = 0; i < inp.size(); ++i )
            inp[ i ].inst->set_out_bt_hint( inp[ i ].nout, reinterpret_cast<const BaseType **>( additionnal_data )[ 0 ] );
        break;
    // case Op_...:
        #define DECL_IR_TOK( INST ) case CppInst::Id_Op_##INST:
        #include "../Ir/Decl_BinaryHomoOperations.h"
        #include "../Ir/Decl_UnaryHomoOperations.h"
        #undef DECL_IR_TOK
        set_out_bt_hint( 0, reinterpret_cast<const BaseType **>( additionnal_data )[ 0 ] );
        for( int i = 0; i < inp.size(); ++i )
            inp[ i ].inst->set_out_bt_hint( inp[ i ].nout, reinterpret_cast<const BaseType **>( additionnal_data )[ 0 ] );
        break;
    case CppInst::Id_Syscall:
        set_out_bt_hint( 0, bt_Void );
        set_out_bt_hint( 1, ip->bt_ST );
        inp[ 0 ].inst->set_out_bt_hint( inp[ 0 ].nout, bt_Void );
        for( int i = 1; i < inp.size(); ++i )
            inp[ i ].inst->set_out_bt_hint( inp[ i ].nout, ip->bt_ST );
        break;
    case CppInst::Id_PointerOn:
        set_out_bt_hint( 0, ip->bt_ST );
        break;
    case CppInst::Id_Conv:
        set_out_bt_hint( 0, reinterpret_cast<const BaseType **>( additionnal_data )[ 0 ] );
        inp[ 0 ].inst->set_out_bt_hint( inp[ 0 ].nout, reinterpret_cast<const BaseType **>( additionnal_data )[ 1 ] );
        break;
    case CppInst::Id_Rand:
        set_out_bt_hint( 0, bt_PI64, true );
        break;
    case CppInst::Id_WhileOut: {
        CppExpr ch = inp.back();
        ch.inst->set_out_bt_hint( ch.nout, bt_Bool );
        break;
    }
    case CppInst::Id_IfInp: {
        set_out_bt_hint( 0, bt_Bool );
        break;
    }
    case CppInst::Id_SetVal: {
        const SetValData *sd = rcast( additionnal_data );
        inp[ 2 ].inst->set_out_bt_hint( inp[ 2 ].nout, get_bt( sd->off_size_in_bits, true, false ) );
        break;
    }
    }
}


void CppInst::write_to_stream( Stream &os ) const {
    // particular cases
    if ( inst_id == CppInst::Id_Cst ) {
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
        #define DECL_INST( INST ) case CppInst::Id_##INST: os << #INST; break;
        #include "../Inst/DeclInst.h"
        #undef DECL_INST
        default: ERROR( "?" );
    }

    if ( inp.size() ) {
        os << "(";
        for( int i = 0; i < inp.size(); ++i )
            os << ( i ? "," : "" ) << inp[ i ];
        os << ")";
    }
}

static bool inlinable( const CppInst::Out &out ) {
    return out.parents.size() == 1 and
            out.parents[ 0 ].inst->inst_id != Inst::Id_Slice and
            out.parents[ 0 ].inst->inst_id != Inst::Id_WhileInst and
            out.parents[ 0 ].inst->inst_id != Inst::Id_IfOut and
            ( out.parents[ 0 ].inst->inst_id != Inst::Id_If or out.parents[ 0 ].ninp == 0 ) and
            ( out.parents[ 0 ].inst->inst_id != Inst::Id_SetVal or out.parents[ 0 ].ninp != 0 )
            ;
}

static bool declable( CppExpr expr ) {
    return expr.inst->out[ expr.nout ].bt_hint and expr.inst->out[ expr.nout ].bt_hint != bt_Void;
}

/// binary operation
void CppInst::write_code_bin_op( CppCompiler *cc, int prec, const char *op_str, int prec_op ) {
    if ( prec >= 0 or not inlinable( out[ 0 ] ) ) {
        // can we reuse a reg ?
        if ( same_op( prec_op ) and out[ 0 ].num < 0 ) {
            for( int i = 0; i < 2; ++i ) {
                int n = inp[ i ].inst->out[ inp[ i ].nout ].num;
                if ( n >= 0 and cc->to_be_used[ n ] == 1 ) {
                    out[ 0 ].num = n;
                    break;
                }
            }
        }

        if ( prec < 0 ) cc->on.write_beg() << decl( cc, 0 );
        if ( prec >= prec_op ) cc->os << "( ";
        cc->os << inp[ 0 ].inst->inst( cc, inp[ 0 ].nout, prec_op ) << op_str << inp[ 1 ].inst->inst( cc, inp[ 1 ].nout, prec_op );
        if ( prec >= prec_op ) cc->os << " )";
        if ( prec < 0 ) cc->on.write_end( ";" );
    } else
        inlined = true;
}

void CppInst::propagate_reg_num( int nout, int num ) {
    if ( out[ nout ].num >= 0 )
        return;

    switch ( inst_id ) {
    case Inst::Id_SetVal:
        inp[ 0 ].inst->propagate_reg_num( inp[ 0 ].nout, num );
        break;
    default:
        out[ nout ].num = num;
        break;
    }
}

bool CppInst::conv( int &val, int nout ) const {
    if ( inst_id == Inst::Id_Cst ) {
        int *s = rcast( additionnal_data );
        if ( *s <= 8 ) {
            SI8 *v = rcast( additionnal_data + 4 );
            val = *v;
            return true;
        }
        if ( *s <= 16 ) {
            SI16 *v = rcast( additionnal_data + 4 );
            val = *v;
            return true;
        }
        if ( *s == 32 ) {
            int *v = rcast( additionnal_data + 4 );
            val = *v;
            return true;
        }
        if ( *s == 64 ) {
            SI64 *v = rcast( additionnal_data + 4 );
            if ( *v <= std::numeric_limits<int>::max() ) {
                val = *v;
                return true;
            }
        }
    }
    return false;
}


void CppInst::write_code( CppCompiler *cc, int prec ) {
    switch ( inst_id ) {
    case CppInst::Id_Cst:
        if ( prec >= 0 )
            out[ 0 ].bt_hint->write_to_stream( *cc->os.stream, additionnal_data + sizeof( int ) );
        else if ( not inlinable( out[ 0 ] ) or not out[ 0 ].bt_hint->c_type() ) {
            // declaration
            int num = out[ 0 ].num;
            if ( num < 0 and declable( CppExpr( this, 0 ) ) )
                cc->os << decl( cc, 0, false ); // << ";";
            // value
            int s = out[ 0 ].bt_hint->size_in_bytes();
            out[ 0 ].bt_hint->write_c_definition(
                        *cc->os.stream,
                        "R" + to_string( out[ 0 ].num ),
                        additionnal_data + sizeof( int ),
                        additionnal_data + sizeof( int ) + s,
                        cc->on.nsp
                     );
        } else
            inlined = true;
        break;
    case CppInst::Id_Rand:
        cc->add_include( "stdlib.h" );
        cc->on << decl( cc, 0 ) << "rand();";
        break;
    case CppInst::Id_Conv:
        if ( prec >= 0 ) {
            cc->bt_to_decl.insert( out[ 0 ].bt_hint );
            cc->os << *out[ 0 ].bt_hint << "( " << disp( cc, inp[ 0 ] ) << " )";
        } else if ( not inlinable( out[ 0 ] ) )
            cc->on << decl( cc, 0 ) << disp( cc, inp[ 0 ] ) << ";";
        else
            inlined = true;
        break;
    case CppInst::Id_Phi:
        if ( prec >= 0 or not inlinable( out[ 0 ] ) ) {
            if ( prec < 0 )
                cc->on.write_beg() << decl( cc, 0 );
            cc->os << disp( cc, inp[ 0 ], PREC_phi ) << " ? "
                   << disp( cc, inp[ 1 ], PREC_phi ) << " : "
                   << disp( cc, inp[ 2 ], PREC_phi );
            if ( prec < 0 )
                cc->on.write_end( ";" );
        } else
            inlined = true;
        break;
    case CppInst::Id_Syscall:
        cc->add_include( "unistd.h" );

        cc->on.write_beg();
        if ( out[ 1 ].parents.size() )
            cc->os << decl( cc, 1 );
        cc->os << "syscall( ";
        for( int i = 1; i < inp.size(); ++i ) {
            if ( i > 1 )
                cc->os << ", ";
            cc->os << disp( cc, inp[ i ], 0 );
        }
        cc->on.write_end( " );" );
        break;

    case CppInst::Id_If: {
        // num reg in IfInp
        for( int ninp = 0; ninp < inp.size(); ++ninp ) {
            if ( declable( inp[ ninp ] ) ) {
                cc->to_be_used[ inp[ ninp ].inst->out[ inp[ ninp ].nout ].num ] = 0;
                for( int n = 2; n < 4; ++n )
                    if ( ninp < ext[ n ]->out.size() )
                        ext[ n ]->out[ ninp ].num = inp[ ninp ].inst->out[ inp[ ninp ].nout ].num;
            }
        }

        // instructions to exec
        Vec<CppInst *> va[ 2 ];
        for( int n = 0; n < 2; ++n )
            va[ n ] << ext[ n ];

        // ok code
        std::ostringstream sok;
        cc->push_stream( &sok );
        cc->on.nsp += 4;
        cc->output_code_for( va[ 0 ] );
        cc->on.nsp -= 4;
        cc->pop_stream();

        // ko code
        std::ostringstream sko;
        cc->push_stream( &sko );
        cc->on.nsp += 4;
        cc->output_code_for( va[ 1 ] );
        cc->on.nsp -= 4;
        cc->pop_stream();

        //

        if ( sok.str().size() ) {
            cc->on << "if ( " << disp( cc, inp[ 0 ] ) << " ) {";
            cc->os << sok.str();
            if ( sko.str().size() ) {
                cc->on << "} else {";
                cc->os << sko.str();
            }
            cc->on << "}";
        } else {
            cc->on << "if ( not " << disp( cc, inp[ 0 ], PREC_not ) << " ) {";
            cc->os << sko.str();
            cc->on << "}";
        }

        for( int nout = 0; nout < out.size(); ++nout )
            out[ nout ].num = ext[ 0 ]->inp[ nout ].inst->out[ ext[ 0 ]->inp[ nout ].nout ].num;
        break;
    }

    case CppInst::Id_IfOut: {
        CppInst *ii = ext_parent;
        for( int ninp = 0; ninp < inp.size(); ++ninp ) {
            if ( declable( inp[ ninp ] ) ) {
                if ( ii->out[ ninp ].num < 0 ) {
                    int r = inp[ ninp ].inst->out[ inp[ ninp ].nout ].num;
                    ii->out[ ninp ].num = r;
                    // propagation from the other IfOut
                    CppExpr oc = ii->ext[ 1 ]->inp[ ninp ];
                    oc.inst->propagate_reg_num( oc.nout, r );
                } else if ( ii->out[ ninp ].num != inp[ ninp ].inst->out[ inp[ ninp ].nout ].num )
                    cc->on << "R" << ii->out[ ninp ].num  << " = R" << inp[ ninp ].inst->out[ inp[ ninp ].nout ].num << ";";
            }
        }
        if ( write_break )
            cc->on << "break;";
        break;
    }

    case CppInst::Id_IfInp: {
        break;
    }

    case CppInst::Id_WhileInst: {
        const int *corr_inp = rcast( additionnal_data );
        for( int i = 0; i < out.size(); ++i ) {
            if ( declable( ext[ 0 ]->inp[ i ] ) ) {
                int ninp = corr_inp[ i ];
                if ( ninp >= 0 ) {
                    if ( cc->to_be_used[ inp[ ninp ].inst->out[ inp[ ninp ].nout ].num ] == 1 ) {
                        out[ i ].num = inp[ ninp ].inst->out[ inp[ ninp ].nout ].num;
                        cc->to_be_used[ inp[ ninp ].inst->out[ inp[ ninp ].nout ].num ] = 0; // because we will use it DURING the while
                    } else
                        cc->on << decl( cc, i ) << disp( cc, inp[ ninp ] ) << ";";
                } else
                    cc->on << decl( cc, i, false ) << ";";
            }
        }

        //
        cc->on << "while ( true ) {";

        cc->on.nsp += 4;
        Vec<CppInst *> out;
        for( const CppExpr &ch : ext[ 0 ]->inp )
            out << ch.inst;
        cc->output_code_for( out );

        int cont_val;
        CppExpr cont = ext[ 0 ]->inp.back();
        if ( cont.conv( cont_val ) ) {
            if ( not cont_val )
                cc->on << "break;";
        } else {
            cc->on << "if ( not " << disp( cc, cont, PREC_not ) << " )";
            cc->on.nsp += 4;
            cc->on << "break;";
            cc->on.nsp -= 4;
        }

        cc->on.nsp -= 4;
        cc->on << "}";
        break;
    }

    case CppInst::Id_WhileInp: {
        const int *corr_inp = rcast( ext_parent->additionnal_data );
        for( int i = 0; i < out.size(); ++i ) {
            for( int j = 0; ; ++j ) {
                if ( j == ext_parent->out.size() ) {
                    // variable not modified during the loop
                    CppExpr ch = ext_parent->inp[ i ];
                    out[ i ].num = ch.inst->out[ ch.nout ].num;
                    break;
                }
                if ( corr_inp[ j ] == i ) {
                    out[ i ].num = ext_parent->out[ j ].num;
                    break;
                }
            }
        }
        break;
    }

    case CppInst::Id_WhileOut: {
        for( int i = 0; i < inp.size() - 1; ++i ) {
            if ( ext_parent->out[ i ].num != inp[ i ].inst->out[ inp[ i ].nout ].num )
                cc->on << "R" << ext_parent->out[ i ].num << " = "
                       << disp( cc, inp[ i ], 0 ) << ";";
        }
        break;
    }

    // bin bool
    case CppInst::Id_Op_or    : write_code_bin_op( cc, prec, " or " , PREC_or     ); break;
    case CppInst::Id_Op_and   : write_code_bin_op( cc, prec, " and ", PREC_and    ); break;
    case CppInst::Id_Op_equ   : write_code_bin_op( cc, prec, " == " , PREC_equ    ); break;
    case CppInst::Id_Op_neq   : write_code_bin_op( cc, prec, " != " , PREC_neq    ); break;
    case CppInst::Id_Op_sup   : write_code_bin_op( cc, prec, " > "  , PREC_sup    ); break;
    case CppInst::Id_Op_inf   : write_code_bin_op( cc, prec, " < "  , PREC_inf    ); break;
    case CppInst::Id_Op_sup_eq: write_code_bin_op( cc, prec, " >= " , PREC_sup_eq ); break;
    case CppInst::Id_Op_inf_eq: write_code_bin_op( cc, prec, " <= " , PREC_inf_eq ); break;

    case CppInst::Id_Op_add: write_code_bin_op( cc, prec, " + ", PREC_add ); break;
    case CppInst::Id_Op_sub: write_code_bin_op( cc, prec, " - ", PREC_sub ); break;
    case CppInst::Id_Op_mul: write_code_bin_op( cc, prec, " * ", PREC_mul ); break;
    case CppInst::Id_Op_div: write_code_bin_op( cc, prec, " / ", PREC_div ); break;
    case CppInst::Id_Op_mod: write_code_bin_op( cc, prec, " % ", PREC_mod ); break;

    case CppInst::Id_SetVal: {
        // size in bits ?
        SetValData *sf = rcast( additionnal_data );
        if ( not sf->size_is_in_bytes )
            TODO;

        // -> can we reuse the previous reg ?
        if ( cc->to_be_used[ inp[ 0 ].inst->out[ inp[ 0 ].nout ].num ] == 1 ) {
            out[ 0 ].num = inp[ 0 ].inst->out[ inp[ 0 ].nout ].num;
            cc->bt_to_decl.insert( inp[ 1 ].inst->out[ inp[ 1 ].nout ].bt_hint );

            int off;
            if ( inp[ 2 ].conv( off ) and off == 0 ) {
                cc->on << "(" << *inp[ 1 ].inst->out[ inp[ 1 ].nout ].bt_hint << " &)"
                       << disp( cc, CppExpr( this, 0 ) ) << " = "
                       << disp( cc, inp[ 1 ] ) << ";";
            } else {
                cc->on << "*(" << *inp[ 1 ].inst->out[ inp[ 1 ].nout ].bt_hint << " *)( "
                       << disp( cc, CppExpr( this, 0 ), 0, true ) << " + "
                       << disp( cc, inp[ 2 ], PREC_add ) << " ) = "
                       << disp( cc, inp[ 1 ] ) << ";";
            }
        } else {
            ERROR( "TODO: setval without reuse" );
        }
        break;
    }

    case CppInst::Id_Slice: {
        int beg = *(int *)( additionnal_data + 0 * sizeof( int ) );
        int end = *(int *)( additionnal_data + 1 * sizeof( int ) );
        if ( beg % 8 or end % 8 )
            TODO;
        if ( beg ) {
            cc->on << decl( cc, 0 ) << "*(const " << *out[ 0 ].bt_hint << " *)( " << disp( cc, inp[ 0 ], 0, true ) << " + " << beg / 8 << " );";
        } else
            cc->on << decl( cc, 0 ) << "(const " << *out[ 0 ].bt_hint << " &)" << disp( cc, inp[ 0 ] ) << ";";
        break;
    }

    case CppInst::Id_PointerOn: {
        if ( prec >= 0 or not inlinable( out[ 0 ] ) ) {
            if ( prec < 0 )
                cc->on.write_beg() << decl( cc, 0 );
            cc->os << disp( cc, inp[ 0 ], PREC_phi, true );
            if ( prec < 0 )
                cc->on.write_end( ";" );
        }
        // always true to avoid mod of the pointed variable before destruction of the pointer
        inlined = true;
        break;
    }

    default:
        #define DECL_INST( INST ) if ( inst_id == CppInst::Id_##INST ) std::cout << #INST << std::endl;
        #include "../Inst/DeclInst.h"
        #undef DECL_INST
        // TODO;
    }
}

bool CppInst::while_has_only_if_with_cond_as_inp() {
    if ( inp.size() < 1 )
        return false;
    CppInst *wout = ext[ 0 ];
    for( int i = 0; i < inp.size(); ++i ) {
        if ( wout->inp[ i ].inst->inst_id != Inst::Id_If )
            return false;
        if ( wout->inp[ i ].inst->inp[ 0 ] != wout->inp.back() )
            return false;
    }
    return true;
}

void CppInst::mark_children_wo_ext() {
    if ( op_id == cur_op_id )
        return;
    op_id = cur_op_id;

    for( int i = 0, nch = inp.size(); i < nch; ++i )
        inp[ i ].inst->mark_children_wo_ext();
}

void CppInst::get_insts_rec( Vec<CppInst *> &res, int id ) {
    if ( op_id == cur_op_id )
        return;
    op_id = cur_op_id;

    if ( inst_id == id )
        res << this;

    for( int i = 0, nch = inp.size(); i < nch; ++i )
        inp[ i ].inst->get_insts_rec( res, id );
    for( int i = 0; i < ext_ds; ++i )
        ext[ i ]->get_insts_rec( res, id );
}

int CppInst::display_graph( const Vec<CppInst *> &res, DotDisp *omd, const char *filename ) {
    ++cur_op_id;

    std::ofstream f( filename );
    f << "digraph Instruction {";
    if ( full_dot )
        f << "  node [shape = record];";
    else
        f << "  node [shape = none];";
    for( int i = 0; i < res.size(); ++i )
        res[ i ]->write_graph_rec( f, omd );
    f << "}";
    f.close();

    return system( ( "dot -Tps " + std::string( filename ) + " > " + std::string( filename ) + ".eps && gv " + std::string( filename ) + ".eps" ).c_str() );
}

void CppInst::write_graph_rec( Stream &os, DotDisp *omd ) const {
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
    if ( omd )
        omd->disp( os, this );
    // os << '(' << ( op_mp != 0 ) << ')';
    if ( full_dot ) {
        for( int i = 0; i < out.size(); ++i ) {
            os << "|<f" << i << ">";
            if ( out[ i ].bt_hint )
                os << *out[ i ].bt_hint;
            else
                os << "o";
        }
        for( int i = 0; i < inp.size(); ++i )
            os << "|<f" << out.size() + i << ">i";
    }
    os << "\"];\n";

    // children
    for( int i = 0, n = inp.size(); i < n; ++i ) {
        const CppExpr &ch = inp[ i ];
        if ( full_dot )
            os << "    node" << this << ":f" << out.size() + i << " -> node" << ch.inst << ":f" << ch.nout << ";\n";
        else
            os << "    node" << this << " -> node" << ch.inst << ";\n";
        if ( ch.inst )
            ch.inst->write_graph_rec( os, omd );
    }

    // ext
    for( int i = 0, n = ext_disp_size(); i < n; ++i ) {
        const CppInst *ch = ext[ i ];
        os << "    node" << this << " -> node" << ch << " [color=\"green\"];\n";
        if ( ch ) {
            os << "    subgraph cluster_" << ch <<" {\ncolor=yellow;\nstyle=dotted;\n";
            ch->write_graph_rec( os, omd );
            os << "    }\n";
       }
    }

    // parents
    //    for( int nout = 0; nout < out_size(); ++nout ) {
    //        VPar &p = parents( nout );
    //        for( int i = 0; i < p.size(); ++i ) {
    //            os << "    node" << p[ i ] << " -> node" << this << ":f" << nout << " [color=\"red\"];\n";
    //            // parents[ i ]->write_graph_rec( os );
    //        }
    //    }
}

int CppInst::ext_disp_size() const {
    return ext_ds >= 0 ? ext_ds : ext.size();
}
