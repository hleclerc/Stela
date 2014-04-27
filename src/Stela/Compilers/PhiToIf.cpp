#include "../System/SplittedVec.h"
#include "CppCompiler.h"
#include "CppInst.h"
#include "PhiToIf.h"
#include <map>

struct PhiToIfInfo {
    int nb_phi_children;
    bool ok, ko, ext;
};

#define INFO( A ) reinterpret_cast<PhiToIfInfo *>( ( A )->op_mp )


struct PhiToIf {
    struct DispWithOkKoExt : CppInst::DotDisp {
        void disp( Stream &os, const CppInst *inst ) {
            os << " ";
            if ( INFO( inst )->ko )
                os << "K";
            if ( INFO( inst )->ko )
                os << "O";
            if ( INFO( inst )->ext )
                os << "E";
        }
    };

    PhiToIf( const Vec<CppInst *> &outputs, CppCompiler *cc ) : res( outputs ), cc( cc ) {
        // get info
        ++CppInst::cur_op_id;
        for( int i = 0; i < res.size(); ++i )
            make_info_rec( res[ i ] );
    }

    void exec() {
        Vec<Vec<CppInst *> > todo;
        todo << res;

        //
        while ( todo.size() ) {
            Vec<CppInst *> out = todo.back();
            todo.pop_back();

            while ( iter( out ) )
                ;

            ++CppInst::cur_op_id;
            for( CppInst *inst : out )
                add_ext_out_to_todo( todo, inst );
        }
    }

    void add_ext_out_to_todo( Vec<Vec<CppInst *> > &todo, CppInst *inst ) {
        if ( inst->op_id == CppInst::cur_op_id )
            return;
        inst->op_id = CppInst::cur_op_id;

        for( int i = 0; i < inst->ext_ds; ++i )
            todo << Vec<CppInst *>( inst->ext[ i ] );

        for( int i = 0; i < inst->inp.size(); ++i )
            add_ext_out_to_todo( todo, inst->inp[ i ].inst );
    }

    bool iter( Vec<CppInst *> &out ) {
        // set ok, ko, ext to false
        PI64 pti_op_id = ++CppInst::cur_op_id;
        for( CppInst *inst : out )
            update_info_rec( inst );

        // get conditions
        ++CppInst::cur_op_id;
        Vec<CppExpr> cond_list;
        for( CppInst *inst : out )
            get_conditions_rec( cond_list, pti_op_id, inst );

        PRINT( cond_list.size() );
        if ( not cond_list.size() )
            return false;

        CppExpr best_cond;
        int max_np = -1;
        for( CppExpr cond : cond_list ) {
            int np = 0;
            for( int nout = 0; nout < cond.inst->out.size(); ++nout )
                for( const CppInst::Out::Parent &p : cond.inst->out[ nout ].parents )
                    np += p.inst->op_id >= pti_op_id and p.inst->inst_id == CppInst::Id_Phi and p.ninp == 0;
            if ( max_np < np ) {
                max_np = np;
                best_cond = cond;
            }
        }

        PRINT( cond_list );
        PRINT( best_cond );
        return false;

        /*
        // make a substitution for the first encoutered condition without phi nodes
        // (to be optimized)
        for( int ok_part = 0; ok_part < 2; ++ok_part ) {
            for( int nc = 0; nc < cond_list.size(); ++nc ) {
                const CppExpr &cond = cond_list[ nc ];
                if ( INFO( cond.inst )->nb_phi_children == 0 ) {
                    Vec<CppInst *> phi_nodes;
                    bool partial = false;
                    for( int nout = 0; nout < cond.inst->out.size(); ++nout ) {
                        for( const CppInst::Out::Parent &p : cond.inst->out[ nout ].parents ) {
                            if ( p.inst->op_id >= pti_op_id and p.inst->inst_id == CppInst::Id_Phi and p.ninp == 0 ) {
                                if ( INFO( p.inst )->nb_phi_children == 1 )
                                    phi_nodes << p.inst;
                                else
                                    partial = true;
                            }
                        }
                    }
                    PRINT( cond );

                    if ( not phi_nodes.size() )
                        continue;
                    if ( partial and not ok_part )
                        continue;
                    for( CppInst *phi : phi_nodes )
                        PRINT( INFO( phi )->nb_phi_children );

                    // mark all dependant code (children 1 and 2 of phi nodes)
                    // ok_inp = instructions to be executed if ok
                    // ko_inp = instructions to be executed if ko
                    ++CppInst::cur_op_id;
                    for( int i = 0; i < res.size(); ++i )
                        init_ok_ko_ext_rec( res[ i ] );

                    for( CppInst *phi : phi_nodes ) {
                        mark_ok_rec( phi->inp[ 0 ].inst );
                        mark_ok_rec( phi->inp[ 1 ].inst );

                        mark_ko_rec( phi->inp[ 0 ].inst );
                        mark_ko_rec( phi->inp[ 2 ].inst );
                    }

                    for( CppInst *phi : phi_nodes )
                        INFO( phi )->ext = true;
                    for( int i = 0; i < res.size(); ++i )
                        mark_ext_rec( res[ i ] );

                    //DispWithOkKoExt d;
                    //CppInst::display_graph( res, &d );

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
                    replace_common_exprs( ifout_ok, if_inst, ifinp_ok, ifinp_ko );
                    ++CppInst::cur_op_id;
                    replace_common_exprs( ifout_ko, if_inst, ifinp_ko, ifinp_ok );

                    return true;
                }
            }
        }
        return false;
        */
    }

    /// returns nb child phi node
    void make_info_rec( const CppInst *inst ) {
        if ( inst->op_id == CppInst::cur_op_id )
            return;
        inst->op_id = CppInst::cur_op_id;
        inst->op_mp = info_it.push_back();

        for( int i = 0; i < inst->inp.size(); ++i )
            make_info_rec( inst->inp[ i ].inst );
        for( int i = 0; i < inst->ext.size(); ++i )
            make_info_rec( inst->ext[ i ] );
    }

    /// returns nb child phi node
    int update_info_rec( const CppInst *inst ) {
        if ( inst->op_id == CppInst::cur_op_id )
            return INFO( inst )->nb_phi_children;
        inst->op_id = CppInst::cur_op_id;

        int res = inst->inst_id == CppInst::Id_Phi;
        for( int i = 0; i < inst->inp.size(); ++i )
            res += update_info_rec( inst->inp[ i ].inst );

        INFO( inst )->nb_phi_children = res;
        return res;
    }

    void get_conditions_rec( Vec<CppExpr> &cond_list, PI64 pti_op_id, const CppInst *inst ) {
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
            get_conditions_rec( cond_list, pti_op_id, inst->inp[ i ].inst );
    }

    void mark_ok_rec( const CppInst *inst ) {
        if ( INFO( inst )->ok )
            return;
        INFO( inst )->ok = true;
        INFO( inst )->ext = false;
        for( int i = 0; i < inst->inp.size(); ++i )
            mark_ok_rec( inst->inp[ i ].inst );
    }

    void mark_ko_rec( const CppInst *inst ) {
        if ( INFO( inst )->ko )
            return;
        INFO( inst )->ko = true;
        INFO( inst )->ext = false;
        for( int i = 0; i < inst->inp.size(); ++i )
            mark_ko_rec( inst->inp[ i ].inst );
    }

    void mark_ext_rec( const CppInst *inst ) {
        if ( INFO( inst )->ext )
            return;
        INFO( inst )->ext = true;
        if ( INFO( inst )->ok or INFO( inst )->ko )
            return;

        for( int i = 0; i < inst->inp.size(); ++i )
            mark_ext_rec( inst->inp[ i ].inst );
    }

    void init_ok_ko_ext_rec( const CppInst *inst ) {
        if ( inst->op_id == CppInst::cur_op_id )
            return;
        inst->op_id = CppInst::cur_op_id;
        INFO( inst )->ext = false;
        INFO( inst )->ok = false;
        INFO( inst )->ko = false;

        for( int i = 0; i < inst->inp.size(); ++i )
            init_ok_ko_ext_rec( inst->inp[ i ].inst );
    }

    void replace_common_exprs( CppInst *inst, CppInst *if_inst, CppInst *ifinp, CppInst *ifinp_alt ) {
        if ( inst->op_id == CppInst::cur_op_id )
            return;
        inst->op_id = CppInst::cur_op_id;

        for( int i = 0; i < inst->inp.size(); ++i ) {
            CppExpr &ch = inst->inp[ i ];
            if ( ( INFO( ch.inst )->ok and INFO( ch.inst )->ko ) or INFO( ch.inst )->ext ) {
                for( int n = 0; ; ++n ) {
                    // need to create a new entry in if_inst (leading to a new output in ifinp) ?
                    if ( n == if_inst->inp.size() ) {
                        if_inst->add_child( ch );
                        ifinp->check_out_size( n + 1 );
                        ifinp_alt->check_out_size( n + 1 );
                        inst->set_child( i, CppExpr( ifinp, n ) );
                        break;
                    }
                    // already in input of if_inst ?
                    if ( if_inst->inp[ n ] == ch ) {
                        ifinp->check_out_size( n + 1 );
                        ifinp_alt->check_out_size( n + 1 );
                        inst->set_child( i, CppExpr( ifinp, n ) );
                        break;
                    }
                }
            } else
                replace_common_exprs( inst->inp[ i ].inst, if_inst, ifinp, ifinp_alt );
        }
    }

    CppInst *make_inst( int id, int nb_out = 0 ) {
        CppInst *res = cc->inst_list.push_back( id, nb_out );
        res->op_mp = info_it.push_back();
        // INFO( res )->nb_phi_children = 666;
        return res;
    }

    // input / output
    Vec<CppInst *>              res;
    CppCompiler                *cc;

    // intermediate data
    SplittedVec<PhiToIfInfo,32> info_it;
};

Vec<CppInst *> phi_to_if( const Vec<CppInst *> &outputs, CppCompiler *cc ) {
    PhiToIf pti( outputs, cc );
    pti.exec();
    return pti.res;
}
