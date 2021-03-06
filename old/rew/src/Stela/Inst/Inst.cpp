#include "../System/AssignIfNeq.h"
#include "InstInfo_C.h"
#include "BoolOpSeq.h"
#include "Codegen_C.h"
#include <fstream>
#include "OutReg.h"
#include "Inst.h"
#include "Cst.h"
#include "Op.h"
#include "Ip.h"

PI64 Inst::cur_op_id = 0;

Inst::Inst() {
    ext_par   = 0;
    when      = 0;

    op_id_vis = 0;
    op_id     = 0;
    op_mp     = 0;

    flags     = 0;

    cpt_use   = 0;
}

Inst::~Inst() {
    rem_ref_to_this();
    delete when;
}

void Inst::write_to_stream( Stream &os, int prec ) const {
    write_dot( os );
    if ( inp.size() ) {
        for( int i = 0; i < inp.size(); ++i )
            os << ( i ? "," : "(" ) << inp[ i ];
        os << ")";
    }
}

void Inst::add_dep( const Expr &val ) {
    val->par << Parent{ this, TPAR_DEP };
    dep << val;
}

void Inst::add_inp( const Expr &val ) {
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

void Inst::clear_children() {
    rem_ref_to_this();
    inp.resize( 0 );
    dep.resize( 0 );
    ext.resize( 0 );
}

void Inst::mark_children() {
    if ( op_id == cur_op_id )
        return;
    op_id = cur_op_id;
    for( Expr &e : inp )
        e->mark_children();
    for( Expr &e : ext )
        e->mark_children();
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
    op_mp = res.ptr();
    created << res;
}

bool Inst::get_val( SI32 &val, Type *type ) {
    if ( type == &ip->type_SI32 )
        return get_val( val );
    if ( type == &ip->type_SI64 ) {
        SI64 res;
        if ( not get_val( res ) )
            return false;
        val = res;
        return val == res;
    }
    PRINT( *type );
    TODO;
    return false;
}

int Inst::sb() const {
    return ( size() + 7 ) / 8;
}

int Inst::size_ptd() const {
    PRINT( *this );
    ERROR( "not a ptr" );
    return 0;
}

int Inst::size_out( int nout ) const {
    PRINT( *this );
    ERROR( "not a multiple output var" );
    return 0;
}

bool Inst::get_val( void *dst, int size, int offset, int dst_offset ) const {
    return false;
}

bool Inst::is_a_pointer() const {
    return false;
}

bool Inst::is_a_Select() const {
    return false;
}

bool Inst::is_a_Room() const {
    return false;
}

void Inst::visit( Visitor &v, bool pointed_data, bool want_dep ) {
    if ( op_id == cur_op_id )
        return;
    op_id = cur_op_id;

    v( this );
    if ( pointed_data )
        _visit_pointed_data( v, want_dep );

    for( Expr i : inp )
        i->visit( v, pointed_data, want_dep );
    if ( want_dep )
        for( Expr i : dep )
            i->visit( v, pointed_data, want_dep );
}

void Inst::_visit_pointed_data( Visitor &v, bool want_dep ) {
}

void Inst::inp_type_proposition( Type *type, int ninp ) {
}

void Inst::out_type_proposition( Type *type ) {
    if ( IIC( this )->out_type != type ) {
        IIC( this )->out_type = type;
        for( Parent &p : par )
            if ( p.ninp >= 0 )
                p.inst->inp_type_proposition( type, p.ninp );
    }
}

void Inst::val_type_proposition( Type *type ) {
}

void Inst::update_out_type() {
}


int Inst::display_graph( const Vec<Expr> &outputs, const char *filename ) {
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

    if ( when )
        when->write_to_stream( ss << "\n" );
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
        os << "    node" << this;
        if ( inp.size() > 1 )
            os << ":f" << i;
        os << " -> node" << inp[ i ].ptr() << ";\n";

        if ( inp[ i ] )
            inp[ i ]->write_graph_rec( ext_buf, os );
    }

    // dependencies
    for( int i = 0; i < dep.size(); ++i ) {
        os << "    node" << this << " -> node" << dep[ i ].ptr() << " [style=dotted];\n";
        if ( dep[ i ] )
            dep[ i ]->write_graph_rec( ext_buf, os );
    }

    // ext
    for( int i = 0; i < ext_disp_size(); ++i )
        if ( const Inst *ch = ext[ i ].ptr() )
            ext_buf << ch;
}

int Inst::ext_disp_size() const {
    return ext.size();
}

void Inst::write_to( Codegen_C *cc, int prec ) {
    //if ( prec < 0 )
    //    cc->on.write_beg() << "// " << *when << " ";
    if ( prec >= 0 )
        write_dot( *cc->os );
    //if ( prec < 0 )
    //    cc->on.write_end();
}

void Inst::write_to( Codegen_C *cc, int prec, OutReg *out_reg ) {
    if ( out_reg )
        *cc->os << *out_reg;
    else
        write_to( cc, prec );
}

bool Inst::going_to_write_c_code() {
    return true;
}

void Inst::_add_store_dep_if_necessary( Expr res, Expr fut ) {
}

Expr Inst::_simplified() {
    return 0;
}

Expr Inst::_get_val() {
    return _get_val( size_ptd() );
}

Expr Inst::_get_val( int len ) {
    ERROR( "_get_val works only with pointer type variables" );
    return 0;
}

void Inst::_set_val( Expr val, int len, Rese, Expr cond ) {
    ip->disp_error( "_set_val works only with pointer type variables" );
}

void Inst::_set_val( Expr val, Rese, Expr cond ) {
    return _set_val( val, val->size(), Rese(), cond );
}

Expr Inst::_simp_slice( int off, int len ) {
    if ( off == 0 and len == size() )
        return this;
    return Expr();
}

void Inst::update_when( const BoolOpSeq &cond ) {
    if ( not when )
        when = new BoolOpSeq( cond );
    else if ( not assign_if_neq( *when, *when or cond ) )
        return;

    for( Expr inst : inp )
        inst->update_when( cond );
    for( Expr inst : dep )
        inst->update_when( cond );
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

BoolOpSeq Inst::get_BoolOpSeq() {
    return BoolOpSeq( this, true );
}

