#include "../System/AssignIfNeq.h"
#include "../Codegen/Codegen_C.h"
#include "BoolOpSeq.h"
#include <fstream>
#include "Type.h"
#include "Inst.h"
#include "Ip.h"

PI64 Inst::cur_op_id = 0;

Inst::Inst() {
    ext_par    = 0;
    when       = 0;
    out_reg    = 0;
    new_reg    = false;
    next_sched = 0;
    prev_sched = 0;

    op_id_vis = 0;
    op_id     = 0;
    op_mp     = 0;

    cpt_use   = 0;
    flags     = 0;

    par_ext_sched = 0;

    when = new BoolOpSeq( False() );
}

Inst::~Inst() {
    rem_ref_to_this();
    delete when;
}

void Inst::set( Expr obj, const BoolOpSeq &cond ) {
    if ( type() != ip->type_Error )
        ip->disp_error( "attempting to set an object that is not a pointer" );
}

Expr Inst::get( const BoolOpSeq &cond ) {
    if ( type() == ip->type_Error )
        return ip->error_var();
    return ip->ret_error( "attempting to get the pointed value of an object that is not a pointer" );
}

Expr Inst::get() {
    return get( BoolOpSeq() );
}

Expr Inst::simplified( const BoolOpSeq &cond ) {
    return this;
}

Type *Inst::type() {
    TODO;
    return 0;
}

bool Inst::same_cst( const Inst *inst ) const { return false; }
bool Inst::emas_cst( const Inst *inst ) const { return false; }

int Inst::size() {
    return type()->size();
}

void Inst::write_to_stream( Stream &os, int prec ) {
    //    Type *t = type();
    //    if ( t and t != ip->type_Type ) {
    //        os << *t;
    //        os << "{";
    //    }
    write_dot( os );
    if ( inp.size() ) {
        for( int i = 0; i < inp.size(); ++i )
            os << ( i ? "," : "(" ) << inp[ i ];
        os << ")";
    }
    //    if ( t and t != ip->type_Type )
    //        os << "}";
}

Type *Inst::type( int nout ) {
    PRINT( *this );
    ERROR( "..." );
    return 0;
}

Type *Inst::ptype( int nout ) {
    ERROR( "..." );
    return 0;
}


Type *Inst::ptype() {
    Type *t = type();
    if ( t->orig == ip->class_Ptr )
        return ip->type_from_type_var( t->parameters[ 0 ] );
    PRINT( *t );
    TODO;
    // ip->disp_error( "ptype() on a non pointer type" );
    return ip->type_Error;
}



void Inst::add_dep( const Expr &val ) {
    val->par << Parent{ this, TPAR_DEP };
    dep << val;
}

void Inst::add_inp( const Expr &val ) {
    if ( val )
        val->par << Parent{ this, int( inp.size() ) };
    inp << val;
}

void Inst::add_ext( const Expr &val ) {
    val->ext_par = this;
    ext << val;
}

void Inst::mod_inp( const Expr &val, int num ) {
    if ( inp.size() <= num )
        inp.resize( num + 1 );
    inp[ num ]->par.remove_first_unordered( Parent{ this, num } );
    val->par << Parent{ this, num };
    inp[ num ] = val;
}

void Inst::mod_dep( const Expr &val, Inst *d ) {
    for( int i = 0; i < dep.size(); ++i ) {
        if ( dep[ i ] == d ) {
            d->par.remove_first_unordered( Parent{ this, -1 } );
            val->par << Parent{ this, -1 };
            dep[ i ] = val;
            break;
        }
    }
}

void Inst::rem_dep( Inst *d ) {
    for( int i = 0; i < dep.size(); ++i ) {
        if ( dep[ i ] == d ) {
            d->par.remove_first_unordered( Parent{ this, -1 } );
            dep.remove( i );
            break;
        }
    }
}

void Inst::add_store_dep( Inst *dst ) {
    if ( op_id == cur_op_id )
        return;
    op_id = cur_op_id;
    _mk_store_dep( dst );
    for( Expr &e : inp )
        e->add_store_dep( dst );
}

void Inst::rem_ref_to_this() {
    for( int num = 0; num < inp.size(); ++num )
        if ( inp[ num ] )
            inp[ num ]->par.remove_first_unordered( Parent{ this, num } );
    for( int num = 0; num < dep.size(); ++num )
        if ( dep[ num ] )
            dep[ num ]->par.remove_first_unordered( Parent{ this, TPAR_DEP } );
    for( int num = 0; num < ext.size(); ++num )
        if ( ext[ num ] )
            ext[ num ]->ext_par = 0;
}

void Inst::replace_this_by_inp( int ninp, Vec<Expr> &out ) {
    for( Parent &p : par ) {
        if ( p.ninp >= 0 )
            p.inst->mod_inp( inp[ ninp ], p.ninp );
        else
            p.inst->add_dep( inp[ ninp ] );
    }

    rem_ref_to_this();

    for( Expr &e : out )
        if ( e.inst == this )
            e = inp[ 0 ];
}


int Inst::pointing_to_nout() {
    return -1;
}

Inst *Inst::find_par_for_nout( int nout ) {
    for( Parent &p : par )
        if ( p.ninp == 0 and p.inst->pointing_to_nout() == nout )
            return p.inst;
    return 0;
}

void Inst::mark_children( Vec<Expr> *seq ) {
    if ( op_id == cur_op_id )
        return;
    op_id = cur_op_id;

    if ( seq )
        *seq << this;

    for( Expr &e : inp )
        if ( e )
            e->mark_children( seq );
    for( Expr &e : dep )
        if ( e )
            e->mark_children( seq );
    for( Expr &e : ext )
        if ( e )
            e->mark_children( seq );
}


void Inst::clone( Vec<Expr> &created ) const {
    if ( op_id == cur_op_id )
        return;
    op_id = cur_op_id;

    // clone the children
    for( const Expr &i : inp )
        i->clone( created );
    for( const Expr &i : dep )
        i->clone( created );
    for( const Expr &i : ext )
        i->clone( created );

    // basic clone
    Expr res = forced_clone( created );

    // add cloned children
    for( const Expr &i : inp )
        res->add_inp( reinterpret_cast<Inst *>( i->op_mp ) );
    for( const Expr &i : dep )
        res->add_dep( reinterpret_cast<Inst *>( i->op_mp ) );
    for( const Expr &i : ext )
        res->add_ext( reinterpret_cast<Inst *>( i->op_mp ) );

    // register
    op_mp = res.inst;
    created << res;
}

int Inst::display_graph( Vec<Expr> outputs, const char *filename ) {
    std::ofstream f( filename );
    f << "digraph Instruction {\n";
    f << "  node [shape = record];\n";

    // get the nodes (within clusters)
    ++cur_op_id;
    Vec<Inst *> ext_buf, seq;
    for( int i = 0; i < outputs.size(); ++i )
        outputs[ i ]->write_graph_rec( ext_buf, seq, f );
    for( Inst *ch : ext_buf )
        if ( ch )
            ch->write_sub_graph_rec( seq, f );

    // edges
    for( Inst *i : seq ) {
        // ext
        for( int e = 0; e < i->ext_disp_size(); ++e )
            f << "    node" << i << " -> node" << i->ext[ e ].inst << " [color=\"green\"];\n";

        // sched
        if ( Inst *n = i->next_sched )
            f << "    node" << n << " -> node" << i << " [color=\"yellow\"];\n";

        // parents
        //    for( int i = 0; i < par.size(); ++i ) {
        //        if ( par[ i ].ninp >= 0 and par[ i ].inst->inp.size() > 1 )
        //            os << "    node" << par[ i ].inst << ":f" << par[ i ].ninp << " -> node" << this << " [color=red,style=dotted];\n";
        //        else
        //            os << "    node" << par[ i ].inst << " -> node" << this << " [color=red,style=dotted];\n";
        //    }
        //if ( par_ext_sched )
        //    os << "    node" << par_ext_sched << " -> node" << this << " [color=yellow,style=dotted];\n";
    }


    f << "}";
    f.close();

    return system( ( "dot -Tps " + std::string( filename ) + " > " + std::string( filename ) + ".eps && gv " + std::string( filename ) + ".eps" ).c_str() );
}

void Inst::write_sub_graph_rec( Vec<Inst *> &seq, Stream &os ) {
    os << "    subgraph cluster_" << this <<" {\ncolor=yellow;\nstyle=dotted;\n";
    Vec<Inst *> ext_buf;
    write_graph_rec( ext_buf, seq, os );
    for( Inst *nch : ext_buf )
        if ( nch )
            nch->write_sub_graph_rec( seq, os );
    os << "    }\n";
}

void Inst::write_graph_rec( Vec<Inst *> &ext_buf, Vec<Inst *> &seq, Stream &os ) {
    if ( op_id_vis == cur_op_id )
        return;
    op_id_vis = cur_op_id;
    seq << this;

    // label
    std::ostringstream ss;
    write_dot( ss );

    if ( when )
        when->write_to_stream( ss << "\n" );
    if ( out_reg )
        ss << ".R" << out_reg->num;
    // ss << " " << sched_num;
    //if ( IIC( this )->out_reg )
    //    IIC( this )->out_reg->write_to_stream( ss << " " );

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
        if ( inp[ i ] ) {
            os << "    node" << this;
            if ( inp.size() > 1 )
                os << ":f" << i;
            os << " -> node" << inp[ i ].inst << ";\n";

            inp[ i ]->write_graph_rec( ext_buf, seq, os );
        }
    }

    // dependencies
    for( int i = 0; i < dep.size(); ++i ) {
        if ( dep[ i ] ) {
            os << "    node" << this << " -> node" << dep[ i ].inst << " [style=dotted];\n";
            dep[ i ]->write_graph_rec( ext_buf, seq, os );
        }
    }

    // ext
    for( int i = 0; i < ext_disp_size(); ++i )
        if ( Inst *ch = ext[ i ].inst )
            ext_buf << ch;
}

int Inst::ext_disp_size() const {
    return ext.size();
}

Expr Inst::_simp_repl_bits( Expr off, Expr val ) {
    return (Inst *)0;
}

Expr Inst::_simp_slice( Type *dst, Expr off ) {
    SI32 voff;
    if ( type() == dst and off->get_val( ip->type_SI32, &voff ) and voff == 0 )
        return this;
    return (Inst *)0;
}

void Inst::_mk_store_dep( Inst *dst ) {
}

bool Inst::used_reg_ok_for( CppOutReg *reg, Inst *inst, int dir ) {
    std::map<CppOutReg *,Inst *>::iterator iter = used_regs.find( reg );
    if ( iter != used_regs.end() and iter->second != inst )
        return false;
    // look in children
    if ( dir >= 0 )
        for( Inst *i : ext_sched )
            if ( not i->used_reg_ok_for( reg, inst, 1 ) )
                return false;
    // look in parent
    if ( dir <= 0 )
        for( Inst *i = this; i; i = i->prev_sched )
            if ( Inst *p = i->par_ext_sched )
                return p->used_reg_ok_for( reg, inst, -1 );
    return true;
}

void Inst::add_used_reg( CppOutReg *reg, Inst *inst ) {
    used_regs[ reg ] = inst;
}

void Inst::used_regs_erase( CppOutReg *out_reg ) {
    used_regs.erase( out_reg );
}

int Inst::always_checked() const {
    return false;
}

bool Inst::has_inp_parent() const {
    for( int i = 0; i < par.size(); ++i )
        if ( par[ i ].ninp >= 0 )
            return true;
    return false;
}

bool Inst::has_inp_parent( Inst *inst ) const {
    for( int i = 0; i < par.size(); ++i )
        if ( par[ i ].ninp >= 0 and par[ i ].inst == inst )
            return true;
    return false;
}

int Inst::nb_inp_parents() const {
    int res = 0;
    for( int i = 0; i < par.size(); ++i )
        res += ( par[ i ].ninp >= 0 );
    return res;
}

bool Inst::uninitialized() const {
    return false;
}

bool Inst::is_surdef() const {
    return flags & SURDEF;
}

bool Inst::is_const() const {
    return flags & CONST;
}

bool Inst::is_Select() const {
    return false;
}

bool Inst::get_val( Type *type, void *data ) const {
    return false;
}

int Inst::op_num() const {
    return -1;
}

bool Inst::referenced_more_than_one_time() const {
    return true;
}

void Inst::update_when( const BoolOpSeq &cond ) {
    if ( not assign_if_neq( *when, *when or cond ) )
        return;

    for( Expr inst : inp )
        if ( inst )
            inst->update_when( cond );
    for( Expr inst : dep )
        if ( inst )
            inst->update_when( cond );
}

void Inst::get_constraints() {
}

//void Inst::add_break_and_continue_internal( CC_SeqItemBlock **b ) {
//    ASSERT( ext.size() == 0, "inst with ext should have an add_break_and_continue_internal method" );
//}

bool Inst::will_write_code() const {
    return true;
}

bool Inst::need_a_register() {
    return has_inp_parent();
}

void Inst::codegen_simplification( Vec<Expr> &created, Vec<Expr> &out ) {
}

static void self_max( int &src, int val ) {
    src = std::max( src, val );
}

void Inst::add_same_out( int src_ninp, Inst *dst_inst, int dst_ninp, int level ) {
    self_max( this->same_out[ PortConstraint{ src_ninp, dst_inst, dst_ninp } ], level );
    self_max( dst_inst->same_out[ PortConstraint{ dst_ninp, this, src_ninp } ], level );
}

void Inst::add_diff_out(int src_ninp, Inst *dst_inst, int dst_ninp, int level ) {
    self_max( this->diff_out[ PortConstraint{ src_ninp, dst_inst, dst_ninp } ], level );
    self_max( dst_inst->diff_out[ PortConstraint{ dst_ninp, this, src_ninp } ], level );
}

CppOutReg *Inst::get_inp_reg( int ninp ) {
    return ninp >= 0 and ninp < inp_reg.size() ? inp_reg[ ninp ] : 0;
}

bool Inst::visit_sched( Inst::Visitor &v, bool with_ext, bool forward, Inst *end ) {
    if ( forward ) {
        for( Inst *b = this; b; b = b->next_sched ) {
            if ( b == end )
                return true;
            if ( not v( b ) )
                return false;
            if ( with_ext )
                for( int i = 0; i < b->ext_sched.size(); ++i )
                    if ( not b->ext_sched[ i ]->visit_sched( v, with_ext, forward, end ) )
                        return false;
        }
    } else {
        for( Inst *b = this; b; b = b->prev_sched ) {
            if ( with_ext )
                for( int i = 0; i < b->ext_sched.size(); ++i )
                    if ( not b->ext_sched[ i ]->visit_sched( v, with_ext, forward, end ) )
                        return false;
            if ( b == end )
                return true;
            if ( not v( b ) )
                return false;
        }
    }
    return true;
}

bool Inst::sched_contains( Inst *inst ) {
    if ( this == inst )
        return true;
    for( int i = 0; i < ext_sched.size(); ++i )
        for( Inst *c = ext_sched[ i ]; c; c = c->next_sched )
            if ( c->sched_contains( inst ) )
                return true;
    return false;
}

void Inst::update_sched_num() {
    int num = 0;
    for( Inst *inst = this; inst; inst = inst->next_sched )
        inst->sched_num = num++;
}

int Inst::set_inp_reg( int ninp, CppOutReg *reg ) {
    if ( ninp >= inp_reg.size() ) {
        inp_reg.resize( inp.size(), (CppOutReg *)0 );
        inp_reg[ ninp ] = reg;
        return 1;
    }
    if ( not inp_reg[ ninp ] ) {
        inp_reg[ ninp ] = reg;
        return 1;
    }
    if ( inp_reg[ ninp ] != reg )
        return -1;
    inp_reg[ ninp ] = reg;
    return 0;
}

void Inst::write( Codegen_C *cc ) {
    cc->on.write_beg();
    if ( out_reg )
        out_reg->write( cc, new_reg ) << " = ";
    write_dot( *cc->os );
    cc->on.write_end( ";" );
}

Inst::operator BoolOpSeq() {
    return BoolOpSeq( this, true );
}
