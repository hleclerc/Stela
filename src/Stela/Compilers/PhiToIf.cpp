#include "../System/SplittedVec.h"
#include "CppCompiler.h"
#include "CppInst.h"
#include "PhiToIf.h"
#include <map>

struct PhiToIfInfo {
    int nb_phi_children;
    bool ok, ko;
};

#define INFO( A ) reinterpret_cast<PhiToIfInfo *>( ( A )->op_mp )


struct PhiToIf {
    PhiToIf( const Vec<CppInst *> &outputs, CppCompiler *cc ) : res( outputs ), cc( cc ) {
        // get info (with nb_phi_children)
        pti_op_id = ++CppInst::cur_op_id;
        for( int i = 0; i < res.size(); ++i )
            make_info_rec( res[ i ] );
    }

    void exec() {
        while ( iter() );
    }

    bool iter() {
        // get conditions
        ++CppInst::cur_op_id;
        cond_list.resize( 0 );
        for( int i = 0; i < res.size(); ++i )
            get_conditions_rec( res[ i ] );

        // make a substitution for the first encoutered condition without phi nodes
        // (to be optimized)
        for( int nc = 0; nc < cond_list.size(); ++nc ) {
            const CppExpr &cond = cond_list[ nc ];
            if ( INFO( cond.inst )->nb_phi_children == 0 ) {
                Vec<CppInst *> phi_nodes;
                for( int nout = 0; nout < cond.inst->out.size(); ++nout )
                    for( const CppInst::Out::Parent &p : cond.inst->out[ nout ].parents )
                        if ( p.inst->op_id >= pti_op_id and p.inst->inst_id == CppInst::Id_Phi and p.ninp == 0 )
                            phi_nodes << p.inst;

                // mark all dependant code (children 1 and 2 of phi nodes)
                // ok_inp = instructions to be executed if ok
                // ko_inp = instructions to be executed if ko
                for( CppInst *phi : phi_nodes ) {
                    mark_ok_rec( phi->inp[ 1 ].inst );
                    mark_ko_rec( phi->inp[ 2 ].inst );
                }

                // new expressions
                int nb_out = phi_nodes.size();
                CppInst *if_inst  = make_inst( CppInst::Id_If   , nb_out );
                CppInst *ifinp_ok = make_inst( CppInst::Id_IfInp );
                CppInst *ifinp_ko = make_inst( CppInst::Id_IfInp );
                CppInst *ifout_ok = make_inst( CppInst::Id_IfOut );
                CppInst *ifout_ko = make_inst( CppInst::Id_IfOut );

                if_inst->add_ext( ifout_ok );
                if_inst->add_ext( ifout_ko );
                if_inst->add_ext( ifinp_ok );
                if_inst->add_ext( ifinp_ko );
                if_inst->ext_ds = 2;

                if_inst->add_child( cond );

                for( int n = 0; n < phi_nodes.size(); ++n ) {
                    // phi node outputs -> outputs of If inst
                    res.replace( phi_nodes[ n ], if_inst );
                    for( const CppInst::Out::Parent &p : phi_nodes[ n ]->out[ 0 ].parents )
                        p.inst->set_child( p.ninp, CppExpr( if_inst, n ) );
                    // input of ifout = inputs of phi nodes
                    ifout_ok->add_child( phi_nodes[ n ]->inp[ 1 ] );
                    ifout_ko->add_child( phi_nodes[ n ]->inp[ 2 ] );
                    phi_nodes[ n ]->set_child( 0, CppExpr() );
                    phi_nodes[ n ]->set_child( 1, CppExpr() );
                    phi_nodes[ n ]->set_child( 2, CppExpr() );
                }

                // replace common exprs (ok and ko) to out of ifinp
                ++CppInst::cur_op_id;
                replace_common_exprs( ifout_ok, if_inst, ifinp_ok );
                ++CppInst::cur_op_id;
                replace_common_exprs( ifout_ko, if_inst, ifinp_ko );

                return true;
            }
        }
        return false;
    }

    /// returns nb child phi node
    int make_info_rec( const CppInst *inst ) {
        if ( inst->op_id == CppInst::cur_op_id )
            return INFO( inst )->nb_phi_children;
        inst->op_id = CppInst::cur_op_id;
        inst->op_mp = info_it.push_back();

        int res = inst->inst_id == CppInst::Id_Phi;
        for( int i = 0; i < inst->inp.size(); ++i )
            res += make_info_rec( inst->inp[ i ].inst );
        for( int i = 0; i < inst->ext.size(); ++i )
            res += make_info_rec( inst->ext[ i ] );

        //INFO( inst )->substitution_inp = subs_inp_it.get_room_for( inst->inp_size() );
        //INFO( inst )->substitution_ext = subs_ext_it.get_room_for( inst->ext_size() );
        INFO( inst )->nb_phi_children = res;
        return res;
    }

    void get_conditions_rec( const CppInst *inst ) {
        if ( inst->op_id == CppInst::cur_op_id )
            return;
        inst->op_id = CppInst::cur_op_id;

        INFO( inst )->ok = false;
        INFO( inst )->ko = false;

        for( int nout = 0; nout < inst->out.size(); ++nout )
            for( const CppInst::Out::Parent &p : inst->out[ nout ].parents )
                if ( p.inst->op_id >= pti_op_id and p.inst->inst_id == CppInst::Id_Phi and p.ninp == 0 )
                    cond_list << p.inst->inp[ 0 ];

        for( int i = 0; i < inst->inp.size(); ++i )
            get_conditions_rec( inst->inp[ i ].inst );
        for( int i = 0; i < inst->ext.size(); ++i )
            get_conditions_rec( inst->ext[ i ] );
    }

    void mark_ok_rec( const CppInst *inst ) {
        if ( INFO( inst )->ok )
            return;
        INFO( inst )->ok = true;
        for( int i = 0; i < inst->inp.size(); ++i )
            mark_ok_rec( inst->inp[ i ].inst );
        for( int i = 0; i < inst->ext.size(); ++i )
            mark_ok_rec( inst->ext[ i ] );
    }

    void mark_ko_rec( const CppInst *inst ) {
        if ( INFO( inst )->ko )
            return;
        INFO( inst )->ko = true;
        for( int i = 0; i < inst->inp.size(); ++i )
            mark_ko_rec( inst->inp[ i ].inst );
        for( int i = 0; i < inst->ext.size(); ++i )
            mark_ko_rec( inst->ext[ i ] );
    }

    void replace_common_exprs( CppInst *inst, CppInst *if_inst, CppInst *ifinp ) {
        if ( inst->op_id == CppInst::cur_op_id )
            return;
        inst->op_id = CppInst::cur_op_id;

        for( int i = 0; i < inst->inp.size(); ++i ) {
            CppExpr &ch = inst->inp[ i ];
            if ( INFO( ch.inst )->ok and INFO( ch.inst )->ko ) {
                for( int n = 0; ; ++n ) {
                    // need to create a new entry in if_inst (leading to a new output in ifinp) ?
                    if ( n == if_inst->inp.size() ) {
                        if_inst->add_child( ch );
                        ifinp->check_out_size( n + 1 );
                        inst->set_child( i, CppExpr( ifinp, n ) );
                        break;
                    }
                    // already in input of if_inst ?
                    if ( if_inst->inp[ n ] == ch ) {
                        ifinp->check_out_size( n + 1 );
                        inst->set_child( i, CppExpr( ifinp, n ) );
                        break;
                    }
                }
            } else
                replace_common_exprs( inst->inp[ i ].inst, if_inst, ifinp );
        }
    }

    CppInst *make_inst( int id, int nb_out = 0 ) {
        CppInst *res = cc->inst_list.push_back( id, nb_out );
        res->op_mp = info_it.push_back();
        return res;
    }

    // input / output
    Vec<CppInst *>              res;
    CppCompiler                *cc;

    // intermediate data
    Vec<CppExpr>                cond_list;
    PI64                        pti_op_id; ///<
    SplittedVec<PhiToIfInfo,32> info_it;
};

Vec<CppInst *> phi_to_if( const Vec<CppInst *> &outputs, CppCompiler *cc ) {
    PhiToIf pti( outputs, cc );
    pti.exec();
    return pti.res;
}