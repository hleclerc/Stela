#include "InstInfo_C.h"
#include <fstream>
#include "Inst.h"
#include "Cst.h"
#include "Op.h"
#include "Ip.h"

PI64 Inst::cur_op_id = 0;

Inst::Inst() {
    ext_par   = 0;

    op_id_vis = 0;
    op_id     = 0;
    op_mp     = 0;

    cpt_use   = 0;
}

Inst::~Inst() {
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

void Inst::write_to_stream( Stream &os ) const {
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

void Inst::mod_inp( const Expr &val, int num ) {
    if ( inp.size() <= num )
        inp.resize( num + 1 );
    inp[ num ]->par.remove_first_unordered( Parent{ this, num } );
    val->par << Parent{ this, num };
    inp[ num ] = val;
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

    // basic clone
    Expr res = forced_clone( created );

    // add cloned children
    for( const Expr &i : inp )
        res->add_inp( reinterpret_cast<Inst *>( i->op_mp ) );
    for( const Expr &i : dep )
        res->add_dep( reinterpret_cast<Inst *>( i->op_mp ) );

    if( ext.size() )
        TODO;

    // register
    op_mp = res.ptr();
    created << res;
}

const PI8 *Inst::data_ptr( int offset ) const {
    return 0;
}

bool Inst::is_a_pointer() const {
    return false;
}

void Inst::visit( Visitor &v, bool pointed_data ) {
    if ( op_id == cur_op_id )
        return;
    op_id = cur_op_id;

    v( this );
    if ( pointed_data )
        _visit_pointed_data( v );

    for( Expr i : inp )
        i->visit( v, pointed_data );
}

void Inst::_visit_pointed_data( Visitor &v ) {
}

int Inst::checked_if( Expr cond ) {
    return cond->allow_to_check( this );
}

int Inst::always_checked() const {
    return 0;
}

int Inst::allow_to_check( Expr val ) {
    return this == val.ptr();
}


Type *Inst::out_type_proposition( Codegen_C *cc ) const {
    return 0;
}

Type *Inst::inp_type_proposition( Codegen_C *cc, int ninp ) const {
    return 0;
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
    for( int i = 0; i < ext.size(); ++i )
        if ( const Inst *ch = ext[ i ].ptr() )
            ext_buf << ch;
}

void Inst::_add_store_dep_if_necessary( Expr res, Expr fut ) {
}

Expr Inst::_simplified() {
    return 0;
}

Expr Inst::_get_val() {
    ERROR( "_get_val works only with pointer type variables" );
    return 0;
}

void Inst::_set_val( Expr val ) {
    ip->disp_error( "_set_val works only with pointer type variables" );
}

Expr Inst::_at( int len ) {
    ip->disp_error( "at work only with pointer type expressions" );
    return cst( 0, 0 );
}

void Inst::_update_when_C( Expr cond ) {
    Expr res = op( &ip->type_Bool, &ip->type_Bool, IIC( this )->when, &ip->type_Bool, cond, Op_or_boolean() );
    if ( IIC( this )->when == res )
        return;

    IIC( this )->when = res;

    for( Expr inst : inp )
        inst->_update_when_C( cond );
    for( Expr inst : dep )
        inst->_update_when_C( cond );
}

void Inst::_get_sub_cond_or( Vec<std::pair<Expr,bool> > &sc, bool pos ) {
    sc << std::make_pair( this, pos );
}

void Inst::_get_sub_cond_and( Vec<std::pair<Expr,bool> > &sc, bool pos ) {
    sc << std::make_pair( this, pos );
}


