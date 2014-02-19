#include "../System/SplittedVec.h"
#include "CppCompiler.h"
#include "CppInst.h"
#include "PhiToIf.h"
#include <map>

struct PhiToIfInfo {
    PhiToIfInfo( const CppInst *inst ) : inst( inst ), ok( false ), ko( false ) {}
    //ConstPtr<Inst> *substitution_ext;
    //Expr *substitution_inp;
    const CppInst *inst;
    int nb_phi_children;
    bool ok, ko;
};

#define INFO( A ) reinterpret_cast<PhiToIfInfo *>( ( A )->op_mp )


struct PhiToIf {
    PhiToIf( const Vec<CppInst *> &outputs, CppCompiler *cc ) : cc( cc ) {
        // get info (with nb_phi_children)
        pti_op_id = ++CppInst::cur_op_id;
        for( int i = 0; i < outputs.size(); ++i )
            make_info_rec( outputs[ i ] );

        // get conditions
        ++CppInst::cur_op_id;
        for( int i = 0; i < outputs.size(); ++i )
            get_conditions_rec( outputs[ i ] );

        // make a substitution for the first encoutered condition without phi nodes
        // (to be optimized)
        res = outputs;
        for( int nc = 0; nc < cond_list.size(); ++nc ) {
            const CppExpr &cond = cond_list[ nc ];
            PRINT( cond );
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
                CppInst *if_inst  = cc->inst_list.push_back( CppInst::Id_If   , nb_out );
                CppInst *ifinp_ok = cc->inst_list.push_back( CppInst::Id_IfInp, 0 );
                CppInst *ifinp_ko = cc->inst_list.push_back( CppInst::Id_IfInp, 0 );
                CppInst *ifout_ok = cc->inst_list.push_back( CppInst::Id_IfOut, 0 );
                CppInst *ifout_ko = cc->inst_list.push_back( CppInst::Id_IfOut, 0 );

                if_inst->add_ext( ifout_ok );
                if_inst->add_ext( ifout_ko );

                for( int n = 0; n < phi_nodes.size(); ++n ) {
                    // phi node outputs -> outputs of If inst
                    res.replace( phi_nodes[ n ], if_inst );
                    for( const CppInst::Out::Parent &p : phi_nodes[ n ]->out[ 0 ].parents )
                        p.inst->set_child( p.ninp, CppExpr( if_inst, n ) );
                    // input of ifout = inputs of phi nodes
                    ifout_ok->add_child( phi_nodes[ n ]->inp[ 1 ] );
                    ifout_ko->add_child( phi_nodes[ n ]->inp[ 2 ] );
                    phi_nodes[ n ]->set_child( 1, CppExpr() );
                    phi_nodes[ n ]->set_child( 2, CppExpr() );
                }

                // replace common exprs (ok and ko) to out of ifinp
                ++CppInst::cur_op_id;
                replace_common_exprs( ifout_ok, if_inst, ifinp_ok );
                ++CppInst::cur_op_id;
                replace_common_exprs( ifout_ko, if_inst, ifinp_ko );

                break;
            }
        }
    }

    /// returns nb child phi node
    int make_info_rec( const CppInst *inst ) {
        if ( inst->op_id == CppInst::cur_op_id )
            return INFO( inst )->nb_phi_children;
        inst->op_id = CppInst::cur_op_id;
        inst->op_mp = info_it.push_back( inst );

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

    CppCompiler                *cc;
    Vec<CppInst *>              res;
    Vec<CppExpr>                cond_list;
    PI64                        pti_op_id; ///<
    SplittedVec<PhiToIfInfo,32> info_it;
};

Vec<CppInst *> phi_to_if( const Vec<CppInst *> &outputs, CppCompiler *cc ) {
    PhiToIf pti( outputs, cc );
    return pti.res;
}
