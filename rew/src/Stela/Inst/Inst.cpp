#include "Inst.h"
#include "Cst.h"
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
    if ( cnd )
        cnd->par.remove_first_unordered( Parent{ this, TPAR_CND } );
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

void Inst::set_cnd( const Expr &val ) {
    if ( cnd )
        cnd->par.remove_first_unordered( Parent{ this, TPAR_CND } );
    val->par << Parent{ this, TPAR_CND };
    cnd = val;
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
    cnd->clone( created );

    // basic clone
    Expr res = forced_clone( created );

    // add cloned children
    for( const Expr &i : inp )
        res->add_inp( reinterpret_cast<Inst *>( i->op_mp ) );
    for( const Expr &i : dep )
        res->add_dep( reinterpret_cast<Inst *>( i->op_mp ) );
    cnd->clone( created );

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

int Inst::bval_if( Expr cond ) {
    return cond->allow_to_check( this );
}

int Inst::always_checked() const {
    return 0;
}

int Inst::allow_to_check( Expr val ) {
    return this == val.ptr();
}


Type *Inst::out_type_proposition( CodeGen_C *cc ) const {
    return 0;
}

Type *Inst::inp_type_proposition( CodeGen_C *cc, int ninp ) const {
    return 0;
}

Expr Inst::_simplified() {
    return 0;
}

Expr Inst::_get_val() const {
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

