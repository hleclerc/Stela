#include "../System/SplittedVec.h"
#include "../Inst/Expr.h"
#include "PhiToIf.h"
#include <map>

struct PhiToIfInfo {
    PhiToIfInfo( const Inst *inst ) : inst( inst ), ok( false ), ko( false ) {}
    ConstPtr<Inst> *substitution_ext;
    Expr *substitution_inp;
    int nb_phi_children;
    const Inst *inst;
    bool ok, ko;
};

#define INFO( A ) reinterpret_cast<PhiToIfInfo *>( ( A )->op_mp )


struct PhiToIf {
    PhiToIf( const Vec<ConstPtr<Inst> > &outputs ) {
        // get info (with nb_phi_children)
        pti_op_id = ++Inst::cur_op_id;
        for( int i = 0; i < outputs.size(); ++i )
            make_info_rec( outputs[ i ].ptr() );

        // get conditions
        ++Inst::cur_op_id;
        for( int i = 0; i < outputs.size(); ++i )
            get_conditions_rec( outputs[ i ].ptr() );

        // make a substitution for the first encoutered condition without phi nodes
        // (to be optimized)
        for( int nc = 0; nc < cond_list.size(); ++nc ) {
            const Expr &cond = cond_list[ nc ];
            PRINT( cond );
            if ( INFO( cond.inst )->nb_phi_children == 0 ) {
                // make IfOut inst -> look for ok==0 and ko==0 with children with ok or ko
                Vec<Expr> ok_inp, ko_inp;
                for( int nout = 0; nout < cond.inst->out_size(); ++nout ) {
                    for( const Inst::Out::Parent &p : cond.inst->out_expr( nout ).parents ) {
                        if ( p.inst->op_id >= pti_op_id and p.inst->inst_id() == Inst::Id_Phi and p.ninp == 0 ) {
                            ok_inp << p.inst->inp_expr( 1 );
                            ko_inp << p.inst->inp_expr( 2 );
                        }
                    }
                }
                PRINT( ok_inp );
                PRINT( ko_inp );

                // mark all dependant code (children 1 and 2 of phi nodes)
                for( int nout = 0; nout < cond.inst->out_size(); ++nout ) {
                    for( const Inst::Out::Parent &p : cond.inst->out_expr( nout ).parents ) {
                        if ( p.inst->op_id >= pti_op_id and p.inst->inst_id() == Inst::Id_Phi and p.ninp == 0 ) {
                            mark_ok_rec( p.inst->inp_expr( 1 ).inst.ptr() );
                            mark_ko_rec( p.inst->inp_expr( 2 ).inst.ptr() );
                        }
                    }
                }

                // make a new expr, with [ok or ko] instructions as input

                break;
            }
        }

        PRINT( cond_list.size() );
        ERROR("...");
    }

    /// returns nb child phi node
    int make_info_rec( const Inst *inst ) {
        if ( inst->op_id == Inst::cur_op_id )
            return INFO( inst )->nb_phi_children;
        inst->op_id = Inst::cur_op_id;
        inst->op_mp = info_it.push_back( inst );

        int res = inst->inst_id() == Inst::Id_Phi;
        for( int i = 0; i < inst->inp_size(); ++i )
            res += make_info_rec( inst->inp_expr( i ).inst.ptr() );
        for( int i = 0; i < inst->ext_size(); ++i )
            res += make_info_rec( inst->ext_inst( i ) );

        INFO( inst )->substitution_inp = subs_inp_it.get_room_for( inst->inp_size() );
        INFO( inst )->substitution_ext = subs_ext_it.get_room_for( inst->ext_size() );
        INFO( inst )->nb_phi_children = res;
        return res;
    }

    void get_conditions_rec( const Inst *inst ) {
        if ( inst->op_id == Inst::cur_op_id )
            return;
        inst->op_id = Inst::cur_op_id;

        for( int nout = 0; nout < inst->out_size(); ++nout )
            for( const Inst::Out::Parent &p : inst->out_expr( nout ).parents )
                if ( p.inst->op_id >= pti_op_id and p.inst->inst_id() == Inst::Id_Phi and p.ninp == 0 )
                    cond_list << p.inst->inp_expr( 0 );

        for( int i = 0; i < inst->inp_size(); ++i )
            get_conditions_rec( inst->inp_expr( i ).inst.ptr() );
        for( int i = 0; i < inst->ext_size(); ++i )
            get_conditions_rec( inst->ext_inst( i ) );
    }

    void mark_ok_rec( const Inst *inst ) {
        if ( INFO( inst )->ok )
            return;
        INFO( inst )->ok = true;
        for( int i = 0; i < inst->inp_size(); ++i )
            mark_ok_rec( inst->inp_expr( i ).inst.ptr() );
        for( int i = 0; i < inst->ext_size(); ++i )
            mark_ok_rec( inst->ext_inst( i ) );
    }

    void mark_ko_rec( const Inst *inst ) {
        if ( INFO( inst )->ko )
            return;
        INFO( inst )->ko = true;
        for( int i = 0; i < inst->inp_size(); ++i )
            mark_ko_rec( inst->inp_expr( i ).inst.ptr() );
        for( int i = 0; i < inst->ext_size(); ++i )
            mark_ko_rec( inst->ext_inst( i ) );
    }


    SplittedVec<PhiToIfInfo,32> info_it;
    SplittedVec<ConstPtr<Inst>,32> subs_ext_it;
    SplittedVec<Expr,32> subs_inp_it;
    Vec<ConstPtr<Inst> > res;
    Vec<Expr> cond_list;
    PI64 pti_op_id; ///<
};

Vec<ConstPtr<Inst> > phi_to_if( const Vec<ConstPtr<Inst> > &outputs ) {
    PhiToIf pti( outputs );
    return pti.res;
}
