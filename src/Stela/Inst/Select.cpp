#include "../System/AssignIfNeq.h"
#include "BoolOpSeq.h"
#include "Select.h"
#include "Copy.h"
#include "Ip.h"

/**
  ok, ko, conds (terms of the BoolOpSeq)
*/
struct Select : Inst {
    Select( Vec<Vec<Bool> > pos ) : pos( pos ) {}
    Select() {}

    virtual void write_dot( Stream &os ) const { os << "Select"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new Select( pos ); }
    virtual Type *type() { return inp[ 0 ]->type(); }
    virtual Expr simplified( const BoolOpSeq &cond ) {
        BoolOpSeq bos = get_bos();
        if ( cond.imply( bos ) )
            return inp[ 0 ];
        if ( cond.imply( not bos ) )
            return inp[ 1 ];
        return this;
    }
    virtual void set( Expr obj, const BoolOpSeq &cond ) {
        TODO;
    }
    virtual bool is_Select() const {
        return true;
    }

    virtual Expr get( const BoolOpSeq &cond ) {
        return select( get_bos() - cond, inp[ 0 ]->get( cond ), inp[ 1 ]->get( cond ) );
    }

    BoolOpSeq get_bos() {
        BoolOpSeq res = True();
        for( int i = 0, o = 2; i < pos.size(); ++i ) {
            Vec<BoolOpSeq::Item> *v = res.or_seq.push_back();
            for( int j = 0; j < pos[ i ].size(); ++j, ++o )
                v->push_back( BoolOpSeq::Item{ inp[ o ], pos[ i ][ j ] } );
        }
        return res;
    }
    virtual void update_when( const BoolOpSeq &cond ) {
        if ( not assign_if_neq( *when, *when or cond ) )
            return;

        BoolOpSeq bos = get_bos();
        inp[ 0 ]->update_when( cond and bos );
        inp[ 1 ]->update_when( cond and not bos );
        for( int i = 2; i < inp.size(); ++i )
            inp[ i ]->update_when( cond );

        for( Expr inst : dep )
            inst->update_when( cond );
    }
    virtual void write( Codegen_C *cc ) {
    }
    virtual bool will_write_code() const {
        return false;
    }
    //    virtual void codegen_simplification( Vec<Expr> &created, Vec<Expr> &out ) {
    //        Expr c0 = copy( inp[ 0 ] );
    //        Expr c1 = copy( inp[ 1 ] );
    //        mod_inp( c0, 0 );
    //        mod_inp( c1, 1 );
    //    }
    virtual void get_constraints() {
        // assuming codegen_simplification have added copy children
        inp[ 0 ]->add_same_out( -1, this, -1, COMPULSORY ); // out of inp[ 0 ] <-> out of this
        inp[ 1 ]->add_same_out( -1, this, -1, COMPULSORY ); // out of inp[ 1 ] <-> out of this
    }

    Vec<Vec<Bool> > pos;
};

/**
  conds (terms of the BoolOpSeq)
*/
struct SelectDep : Select {
    SelectDep( Vec<Vec<Bool> > pos ) : Select( pos ) {}
    SelectDep() {}

    virtual void write_dot( Stream &os ) const { os << "SelectDep"; }
    virtual Expr forced_clone( Vec<Expr> &created ) const { return new SelectDep( pos ); }
    virtual Type *type() { return ip->type_Void; }
    virtual Expr simplified( const BoolOpSeq &cond ) {
        BoolOpSeq bos = get_bos();
        if ( cond.imply( bos ) )
            return dep[ 0 ];
        if ( cond.imply( not bos ) )
            return dep[ 1 ];
        return this;
    }
    virtual void update_when( const BoolOpSeq &cond ) {
        if ( not assign_if_neq( *when, *when or cond ) )
            return;

        for( Expr inst : inp )
            inst->update_when( cond );

        BoolOpSeq bos = get_bos();
        dep[ 0 ]->update_when( cond and bos );
        dep[ 1 ]->update_when( cond and not bos );
        for( int i = 2; i < dep.size(); ++i )
            dep[ i ]->update_when( cond );
    }
    virtual void get_constraints() {
    }
};

Expr select( const BoolOpSeq &cond, Expr ok, Expr ko ) {
    if ( not ko ) return ok;
    if ( cond.always( true  ) ) return ok;
    if ( cond.always( false ) ) return ko;
    if ( ok == ko ) return ok;

    Select *res = new Select;
    res->add_inp( ok );
    res->add_inp( ko );
    for( int i = 0; i < cond.or_seq.size(); ++i ) {
        Vec<Bool> *v = res->pos.push_back();
        for( int j = 0; j < cond.or_seq[ i ].size(); ++j ) {
            res->add_inp( cond.or_seq[ i ][ j ].expr );
            *v << cond.or_seq[ i ][ j ].pos;
        }
    }
    return res;
}

Expr select_dep( const BoolOpSeq &cond, Expr ok, Expr ko ) {
    if ( cond.always( true  ) ) return ok;
    if ( cond.always( false ) ) return ko;
    if ( ok == ko ) return ok;

    SelectDep *res = new SelectDep;
    res->add_dep( ok );
    res->add_dep( ko );
    for( int i = 0; i < cond.or_seq.size(); ++i ) {
        Vec<Bool> *v = res->pos.push_back();
        for( int j = 0; j < cond.or_seq[ i ].size(); ++j ) {
            res->add_inp( cond.or_seq[ i ][ j ].expr );
            *v << cond.or_seq[ i ][ j ].pos;
        }
    }
    return res;
}

