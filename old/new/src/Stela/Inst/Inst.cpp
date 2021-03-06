#include "../CodeGen/InstInfo_C.h"
#include <fstream>
#include "Inst.h"
#include "Ip.h"

PI64 Inst::cur_op_id = 0;

Inst::Inst() {
    ext_par   = 0;

    op_id_vis = 0;
    op_id     = 0;
    op_mp     = 0;
}

Inst::~Inst() {
    for( int num = 0; num < inp.size(); ++num )
        if ( inp[ num ] )
            inp[ num ]->par.remove_first_unordered( Parent{ this, num } );
    for( int num = 0; num < dep.size(); ++num )
        if ( dep[ num ] )
            dep[ num ]->par.remove_first_unordered( Parent{ this, -1 } );
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

void Inst::add_var_ptr( Var *var ) {
    write_dot( std::cerr );
    IP_ERROR( "not an Inst based on a Room instance" );
}

void Inst::set( Ptr<Inst> val ) {
    write_dot( std::cerr );
    IP_ERROR( "not an Inst based on a Room instance" );
}

bool Inst::true_if( const ConstPtr<Inst> &cond ) const {
    return cond->rtrue_if( this );
}

bool Inst::rtrue_if( const ConstPtr<Inst> &val ) const {
    return val.ptr() == this;
}

void Inst::add_inp( Ptr<Inst> val ) {
    val->par << Parent{ this, int( inp.size() ) };
    inp << val;
}

void Inst::mod_inp( int num, Ptr<Inst> val ) {
    inp[ num ]->par.remove_first_unordered( Parent{ this, num } );
    val->par << Parent{ this, num };
    inp[ num ] = val;
}

void Inst::add_dep( Ptr<Inst> val ) {
    val->par << Parent{ this, -1 };
    dep << val;
}

void Inst::clone( Vec<Ptr<Inst> > &created ) const {
    if ( op_id == cur_op_id )
        return;
    op_id = cur_op_id;

    // clone the children
    for( Ptr<Inst> i : inp )
        i->clone( created );
    for( Ptr<Inst> i : dep )
        i->clone( created );

    // basic clone
    Ptr<Inst> res = forced_clone( created );

    // add cloned children
    for( Ptr<Inst> i : inp )
        res->add_inp( reinterpret_cast<Inst *>( i->op_mp ) );
    for( Ptr<Inst> i : dep )
        res->add_dep( reinterpret_cast<Inst *>( i->op_mp ) );

    if( ext.size() )
        TODO;

    // register
    op_mp = res.ptr();
    created << res;
}

void Inst::rec_visit( Visitor &visitor, bool want_ext ) {
    if ( op_id == cur_op_id )
        return;
    op_id = cur_op_id;

    visitor( this );

    for( Ptr<Inst> i : inp )
        i->rec_visit( visitor, want_ext );
    for( Ptr<Inst> i : dep )
        i->rec_visit( visitor, want_ext );
    if ( want_ext )
        for( Ptr<Inst> i : ext )
            i->rec_visit( visitor, want_ext );
}

const PI8 *Inst::data_ptr( int offset ) const {
    return 0;
}

bool Inst::is_and() const {
    return false;
}

Ptr<Inst> Inst::snapshot() {
    return simplified( this );
}

void Inst::write_to( CodeGen_C *cc ) const {
    cc->bdef_if_nec( this );
    write_1l_to( cc );
    cc->on.write_end( ";" );
}

Type *Inst::out_type_proposition( CodeGen_C *cc ) const {
    for( const Parent &p : par )
        if ( p.ninp >= 0 )
            if ( Type *res = p.inst->inp_type_proposition( cc, p.ninp ) )
                return res;
    return 0;
}

Type *Inst::inp_type_proposition( CodeGen_C *cc, int ninp ) const {
    return 0;
}

void Inst::add_when_cond( const Ptr<Inst> &cond ) {
    if ( IIC( this )->add_when_possibility( cond ) )
        return;

    for( Ptr<Inst> i : inp )
        i->add_when_cond( cond );
    for( Ptr<Inst> i : dep )
        i->add_when_cond( cond );
}

int Inst::display_graph( const Vec<ConstPtr<Inst> > &outputs, const char *filename ) {
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

Ptr<Inst> Inst::_simplified() {
    return 0;
}

Ptr<Inst> Inst::_pointer_on( int beg, int len ) {
    return 0;
}

void Inst::_remove_cond( Vec<Ptr<Inst> > &cr ) {
}

Ptr<Inst> simplified( Ptr<Inst> val ) {
    if ( Ptr<Inst> res = val->_simplified() )
        return res;
    return val;
}
