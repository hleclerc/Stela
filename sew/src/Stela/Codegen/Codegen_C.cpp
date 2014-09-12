#include "../System/AutoPtr.h"
#include "../Inst/BoolOpSeq.h"
#include "../Inst/GetNout.h"
#include "../Inst/Type.h"
#include "../Inst/Copy.h"
#include "../Inst/If.h"
#include "../Inst/Ip.h"
#include "Codegen_C.h"
#include <limits>

Codegen_C::Codegen_C() : on( &main_os ), os( &main_os ) {
    disp_inst_graph = false;
    disp_inst_graph_wo_phi = false;

    add_include( "<stdint.h>" );
    add_include( "<stdlib.h>" );
    add_include( "<unistd.h>" );
    add_include( "<time.h>" );

    add_prel( "#pragma cpp_flag -std=c++11" );
}

void Codegen_C::add_include( String name ) {
    includes.insert( name );
}

void Codegen_C::add_prel( String data ) {
    preliminaries.push_back_unique( data );
}

void Codegen_C::add_type( Type *type ) {
    types.insert( type );
}

CppOutReg *Codegen_C::new_out_reg( Type *type ) {
    return out_regs.push_back( type, out_regs.size() );
}

void Codegen_C::write_out( Expr expr ) {
    if ( expr->out_reg )
        *os << "R" << expr->out_reg->num;
    else
        *os << "no_out_reg";
}

String Codegen_C::type_to_str( Type *type ) {
    std::ostringstream ss;
    if ( type->aryth )
        ss << *type;
    else if ( type->orig == ip->class_Ptr ) {
        add_type( ip->type_PI8 );
        ss << "TPTR";
    } else {
        ss << "GT" << type->size();
        spec_types[ type->size() ] = ss.str();
    }
    return ss.str();
}

void Codegen_C::write_to( Stream &out ) {
    on.nsp = 4;
    make_code();

    // include
    for( String inc : includes )
        out << "#include " << inc << "\n";

    // types
    //    std::map<Type *,Vec<OutReg *> > orbt;
    //    for( int i = 0; i < out_regs.size(); ++i )
    //        orbt[ out_regs[ i ].type ] << &out_regs[ i ];
    //    for( Type *type : types )
    //        orbt[ type ];

    // arythmetic types
    out << "typedef unsigned char *TPTR;\n";
    if ( types.count( ip->type_Bool ) ) out << "typedef bool Bool;\n";
    if ( types.count( ip->type_SI8  ) ) out << "typedef signed char SI8;\n";
    if ( types.count( ip->type_SI16 ) ) out << "typedef short SI16;\n";
    if ( types.count( ip->type_SI32 ) ) out << "typedef int SI32;\n";
    if ( types.count( ip->type_SI64 ) ) out << "typedef long long SI64;\n";
    if ( types.count( ip->type_PI8  ) ) out << "typedef unsigned char PI8;\n";
    if ( types.count( ip->type_PI16 ) ) out << "typedef unsigned short PI16;\n";
    if ( types.count( ip->type_PI32 ) ) out << "typedef unsigned int PI32;\n";
    if ( types.count( ip->type_PI64 ) ) out << "typedef unsigned long long PI64;\n";
    if ( types.count( ip->type_FP32 ) ) out << "typedef float FP32;\n";
    if ( types.count( ip->type_FP64 ) ) out << "typedef double FP64;\n";

    // specific types
    for( auto iter : spec_types ) {
        out << "struct " << iter.second << " { ";
        out << "unsigned char data[ " << ( iter.first + 7 ) / 8 << " ]; ";
        out << "};\n";
    }

    // preliminaries
    for( String prel : preliminaries )
        out << prel << "\n";

    //
    out << "int main() {\n";
    out << "    srand(time(0));\n";

    // out_regs
    //    for( auto iter : orbt ) {
    //        if ( iter.second.size() ) {
    //            out << "    " << *iter.first << " ";
    //            for( int n = 0; n < iter.second.size(); ++n ) {
    //                if ( n )
    //                    out << ", ";
    //                out << *iter.second[ n ];
    //            }
    //            out << ";\n";
    //        }
    //    }

    out << main_os.str();
    out << "}\n";
}

void Codegen_C::exec() {
    TODO;
}

static Expr cloned( const Expr &val, Vec<Expr> &created ) {
    val->clone( created );
    return reinterpret_cast<Inst *>( val->op_mp );
}

void get_front( Vec<Expr> &front, Expr inst ) {
    if ( inst->op_id == inst->cur_op_id )
        return;
    inst->op_id = inst->cur_op_id;

    int nb_id = 0;
    for( Expr ch : inst->inp )
        nb_id += not ch->when->always( false );
    for( Expr ch : inst->dep )
        nb_id += not ch->when->always( false );

    if ( nb_id ) {
        for( Expr ch : inst->inp )
            if ( not ch->when->always( false ) )
                get_front( front, ch );
        for( Expr ch : inst->dep )
            if ( not ch->when->always( false ) )
                get_front( front, ch );
    } else
        front << inst;
}

// check if all the children have already been scheduled
static bool ready_to_be_scheduled( Expr inst ) {
    // already in the front ?
    if ( inst->op_id >= Inst::cur_op_id - 1 )
        return false;

    // not computed ?
    for( const Expr &ch : inst->inp )
        if ( ch->op_id < Inst::cur_op_id and not ch->when->always( false ) )
            return false;
    for( const Expr &ch : inst->dep )
        if ( ch->op_id < Inst::cur_op_id and not ch->when->always( false ) )
            return false;

    // ok
    return true;
}

struct OutCondFront {
    Expr inp[ 2 ];
    int  num_in_outputs;
};

static Vec<Expr> extract_inst_that_must_be_done_if( std::map<Inst *,OutCondFront> &outputs, std::map<Expr,int> &inputs, Vec<Expr> &front, BoolOpSeq::Item best_item, bool ok ) {
    // inst in the front that must be done
    Vec<Expr> cond_front, res;
    for( int i = 0; i < front.size(); ++i ) {
        if ( front[ i ]->when->can_be_factorized_by( best_item ) ) {
            Expr expr = front[ i ];
            res << expr;
            // mv inst from front to cond_front
            cond_front << expr;
            front.remove_unordered( i-- );
        }
    }


    // expand the front
    Inst *inst;
    while ( cond_front.size() ) {
        // pick a random ready inst
        inst = cond_front.pop_back().inst;

        // say that it's done
        inst->op_id = Inst::cur_op_id;

        // update the front
        for( Inst::Parent &p : inst->par ) {
            // an instruction that cannot be executed under the condition best_item ?
            if ( not p.inst->when->can_be_factorized_by( best_item ) ) {
                // -> it will be computed under an IfInp, parents will use Nout(If,...)
                Inst *to_be_replaced = inst->par.size() == 1 ? p.inst : inst;
                if ( not outputs.count( to_be_replaced ) )
                    outputs[ to_be_replaced ].num_in_outputs = outputs.size() - 1;
                outputs[ to_be_replaced ].inp[ ok ] = inst;
            } else if ( ready_to_be_scheduled( p.inst ) ) {
                // else,
                p.inst->op_id = Inst::cur_op_id - 1; // -> in the front
                cond_front << p.inst;
                res << p.inst;
            }
        }
    }

    // needed inputs (inp of produced inst that does not belong to the if block)
    for( Expr expr : res )
        for( Expr inp : expr->inp )
            if ( not inp->when->can_be_factorized_by( best_item ) )
                if ( not inputs.count( inp ) )
                    inputs[ inp ] = inputs.size() - 1;

    return res;
}

Expr make_if_inp( std::map<Expr,int> &inputs, const Vec<Expr> &inst_that_need_inp ) {
    // make the if_inp instruction
    Vec<Type *> types( Size(), inputs.size() );
    for( const std::pair<Expr,int> &e : inputs )
        types[ e.second ] = const_cast<Inst *>( e.first.inst )->type();
    Expr res = if_inp( types );

    // nout( if_inp, ... ) instruction
    Vec<Expr> if_inp_sel( Size(), types.size() );
    for( int i = 0; i < if_inp_sel.size(); ++i )
        if_inp_sel[ i ] = get_nout( res, i );

    // use the if_inp instruction
    for( Expr e : inst_that_need_inp )
        for( int ninp = 0; ninp < e->inp.size(); ++ninp )
            if ( inputs.count( e->inp[ ninp ] ) )
                e->mod_inp( if_inp_sel[ inputs[ e->inp[ ninp ] ] ], ninp );

    return res;
}

Expr make_if_out( const std::map<Inst *,OutCondFront> &outputs, bool ok ) {
    Vec<Expr> lst( Size(), outputs.size() );
    for( const std::pair<Inst *,OutCondFront> &e : outputs )
        lst[ e.second.num_in_outputs ] = e.second.inp[ ok ];
    return if_out( lst );
}

Inst *Codegen_C::scheduling( Vec<Expr> out ) {
    // update inst->when
    for( Expr inst : out )
        inst->update_when( BoolOpSeq() );


    // get the front
    ++Inst::cur_op_id;
    Vec<Expr> front;
    for( Expr inst : out )
        get_front( front, inst );
    ++Inst::cur_op_id;
    for( Expr inst : front )
        inst->op_id = Inst::cur_op_id;

    // go to the roots
    Inst *beg = 0, *end;
    ++Inst::cur_op_id;
    while ( front.size() ) {
        // try to find an instruction with the same condition set or an inst that is not going to write anything
        Inst *inst = 0;
        for( int i = 0; i < front.size(); ++i ) {
            if ( front[ i ]->when->always( true ) ) {
                inst = front[ i ].inst;
                front.remove_unordered( i );
                break;
            }
        }

        // if not possible to do more without a condition
        if ( not inst ) {
            // try to find the best condition to move forward
            std::map<BoolOpSeq::Item,int> possible_conditions;
            for( int i = 0; i < front.size(); ++i ) {
                Vec<BoolOpSeq::Item> pc = front[ i ]->when->common_terms();
                for( BoolOpSeq::Item &item : pc )
                    ++possible_conditions[ item ];
            }
            int best_score = -1;
            BoolOpSeq::Item best_item;
            for( const std::pair<BoolOpSeq::Item,int> &p : possible_conditions ) {
                if ( best_score < p.second ) {
                    best_score = p.second;
                    best_item = p.first;
                }
            }

            // start the input list with the conditions
            std::map<Inst *,OutCondFront> outputs; // inst to replace -> replacement values + IfOut pos
            std::map<Expr,int> inputs;
            inputs[ best_item.expr ] = 0;

            // get a front of instructions that must be done under the condition `cond`
            best_item.pos = true;
            Vec<Expr> ok_we_inp = extract_inst_that_must_be_done_if( outputs, inputs, front, best_item, best_item.pos );

            best_item.pos = false;
            Vec<Expr> ko_we_inp = extract_inst_that_must_be_done_if( outputs, inputs, front, best_item, best_item.pos );

            //            for( std::pair<Expr,int> i : inputs )
            //                std::cout << "  inp=" << *i.first << " num=" << i.second << std::endl;
            //            for( std::pair<Inst *,OutCondFront> o : outputs )
            //                std::cout << "  to_be_repl=" << *o.first << "\n    ok=" << o.second.inp[ 1 ] << "\n    ko=" << o.second.inp[ 0 ] << "\n    num=" << o.second.num_in_outputs << std::endl;

            Expr if_inp_ok = make_if_inp( inputs, ok_we_inp );
            Expr if_inp_ko = make_if_inp( inputs, ko_we_inp );

            Expr if_out_ok = make_if_out( outputs, 1 );
            Expr if_out_ko = make_if_out( outputs, 0 );

            // complete the If instruction
            Vec<Expr> inp( Size(), inputs.size() );
            for( std::pair<Expr,int> i : inputs )
                inp[ i.second ] = i.first;
            Expr if_expr = if_inst( inp, if_inp_ok, if_inp_ko, if_out_ok, if_out_ko );
            if_expr->when = new BoolOpSeq( True() );

            // use the if instruction
            Vec<Expr> if_out_sel( Size(), outputs.size() );
            for( int i = 0; i < if_out_sel.size(); ++i ) {
                if_out_sel[ i ] = get_nout( if_expr, i );
                if_out_sel[ i ]->when = new BoolOpSeq( True() );
            }

            for( std::pair<Inst *,OutCondFront> o : outputs )
                for( Inst::Parent &p : o.first->par )
                    p.inst->mod_inp( if_out_sel[ o.second.num_in_outputs ], p.ninp );

            // push if_expr in the front
            if_expr->op_id = Inst::cur_op_id - 1;
            front << if_expr;

            continue;
        }

        // register
        inst->op_id = Inst::cur_op_id; // say that it's done
        if ( not beg ) {
            beg = inst;
            end = inst;
        } else {
            end->next_sched = inst;
            inst->prev_sched = end;
            end = inst;
        }

        // update the front
        for( Inst::Parent &p : inst->par ) {
            if ( ready_to_be_scheduled( p.inst ) ) {
                p.inst->op_id = Inst::cur_op_id - 1; // -> in the front
                front << p.inst;
            }
        }
    }

    // delayed operation (ext blocks)
    for( Inst *inst = beg; inst; inst = inst->next_sched ) {
        // schedule sub block (ext instructions)
        for( int ind = 0; ind < inst->ext_disp_size(); ++ind ) {
            Inst *ext_beg = scheduling( inst->ext[ ind ] );
            ext_beg->par_ext_sched = inst;
            inst->ext_sched << ext_beg;
        }

        // add internal break or continue if necessary
        //        CC_SeqItemBlock *b[ s ];
        //        for( int i = 0; i < s; ++i )
        //            b[ i ] = ne->ext[ i ].ptr();
        //        ne->expr->add_break_and_continue_internal( b );
    }

    return beg;
}

//struct DisplayConstraints : CC_SeqItem::Visitor {
//    virtual bool operator()( CC_SeqItemExpr &ce ) {
//        Inst *i = ce.expr.inst;
//        i->write_dot( std::cout );
//        std::cout << " ->\n";
//        for( auto o : i->same_out ) {
//            if ( o.first.src_ninp < 0 )
//                std::cout << "  out ";
//            else
//                std::cout << "  [" << o.first.src_ninp << "] ";
//            o.first.dst_inst->write_dot( std::cout << "== " );
//            if ( o.first.dst_ninp < 0 )
//                std::cout << " out ";
//            else
//                std::cout << "[" << o.first.dst_ninp << "] ";
//            std::cout << "\n";
//        }
//        for( auto o : i->diff_out ) {
//            if ( o.first.src_ninp < 0 )
//                std::cout << "  out ";
//            else
//                std::cout << "  [" << o.first.src_ninp << "] ";
//            o.first.dst_inst->write_dot( std::cout << "!= " );
//            if ( o.first.dst_ninp < 0 )
//                std::cout << " out ";
//            else
//                std::cout << "[" << o.first.dst_ninp << "] ";
//            std::cout << "\n";
//        }
//        return true;
//    }
//};

struct InstAndPort {
    int  ninp_constraint() const { return std::max( _num, -1 ); }
    bool is_an_output() const { return _num <  0; }
    bool is_an_input () const { return _num >= 0; }
    int  nout() const { return -1 - _num; }
    int  ninp() const { return _num; }

    Inst *inst;
    int   _num; ///  ninp if out==false, nout if out==true
};


static bool _assign_port_rec( Vec<InstAndPort> &assigned_ports, Inst *inst, int ninp, CppOutReg *out_reg ) {
    // assignation
    if ( ninp < 0 ) { // output ?
        // if already assigned
        if ( inst->out_reg )
            return inst->out_reg == out_reg;

        // look if cutting an validated edge using out_reg as an output
        if ( inst->reg_to_avoid.count( out_reg ) )
            return false;

        inst->out_reg = out_reg;
        out_reg->provenance << inst;
        for( int i = 0; i < inst->par.size(); ++i )
            if ( inst->par[ i ].ninp >= 0 )
                assigned_ports << InstAndPort{ inst, -1 - i };
    } else {
        int a = inst->set_inp_reg( ninp, out_reg );
        if ( a <= 0 )
            return a == 0;
        assigned_ports << InstAndPort{ inst, ninp };
    }

    // constraint propagation
    for( std::pair<const Inst::PortConstraint,int> &c : inst->same_out )
        if ( c.second == Inst::COMPULSORY and c.first.src_ninp == ninp and
             not _assign_port_rec( assigned_ports, c.first.dst_inst, c.first.dst_ninp, out_reg ) )
            return false;

    // diff_out -> look if this assignation is going to break a constraint
    for( std::pair<const Inst::PortConstraint,int> &c : inst->diff_out ) {
        if ( c.second == Inst::COMPULSORY and c.first.src_ninp == ninp ) {
            if ( c.first.dst_ninp >= 0 ) {
                if ( c.first.dst_ninp >= c.first.dst_inst->inp_reg.size() or
                     c.first.dst_inst->inp_reg[ c.first.dst_ninp ] == out_reg )
                    return false;
            } else {
                if ( c.first.dst_inst->out_reg == out_reg )
                    return false;
            }
        }
    }

    return true; // OK
}

static void clear_port( InstAndPort port ) {
    if ( port.is_an_output() ) {
        port.inst->out_reg->provenance.pop_back();
        port.inst->out_reg = 0;
    } else
        port.inst->inp_reg[ port.ninp() ] = 0;
}

static bool assign_port_rec( Vec<InstAndPort> &assigned_ports, Inst *inst, int ninp, CppOutReg *out_reg ) {
    Vec<InstAndPort> assigned_ports_trial;

    // ok ?
    if ( _assign_port_rec( assigned_ports_trial, inst, ninp, out_reg ) ) {
        assigned_ports.append( assigned_ports_trial );
        return true;
    }

    // else, undo out_reg assignations
    for( InstAndPort &a : assigned_ports_trial )
        clear_port( a );
    return false;
}

//static bool insert_copy_inst_before( Vec<InstAndPort> &assigned_ports, Inst *inst, Inst *start, const char *reason ) {
//    PRINT( "BEFORE" );
//    PRINT( reason );
//    return false;

//    // _main_block->display_graphviz();

//    CC_SeqItemBlock *par_blk = start->cc_item_expr->parent_block;

//    // new Expr and CC_SeqItemExpr
//    Expr n_expr = copy( start );
//    CC_SeqItemExpr *n_seqi = new CC_SeqItemExpr( n_expr, par_blk );
//    if ( not assign_port_rec( assigned_ports, n_expr.inst, 0, start->out_reg ) )
//         ERROR( "weird" );

//    // position of start inst (inst will be inserted after, and in the same block)
//    int pos_start = 0;
//    while ( par_blk->seq[ pos_start ].ptr() != start->cc_item_expr )
//        ++pos_start;

//    // insertion in block
//    for( int i = pos_start + 1; ; ++i ) {
//        // we're at the end of the block
//        if ( i == par_blk->seq.size() ) {
//            par_blk->seq << n_seqi;
//            break;
//        }

//        //
//        if ( par_blk->seq[ i ]->contains( inst->cc_item_expr ) ) {
//            par_blk->seq.insert( i, n_seqi );
//            break;
//        }
//    }

//    // replace start->out by nexpt->out in expressions that use start->out
//    // (if placed after copy)
//    struct ModInp : public CC_SeqItemExpr::Visitor {
//        virtual bool operator()( CC_SeqItemExpr &ce ) {
//            for( int ninp = 0; ninp < ce.expr->inp.size(); ++ninp ) {
//                if ( ce.expr->inp[ ninp ].inst == start )
//                    ce.expr->mod_inp( n_expr, ninp );
//            }
//            return true;
//        }
//        Inst *start;
//        Expr n_expr;
//    };
//    ModInp mod_inp;
//    mod_inp.start  = start;
//    mod_inp.n_expr = n_expr;
//    n_seqi->following_visit( mod_inp ); // hum. Leads to O(n^2)

//    // always return false (for convenience)
//    return false;
//}

///// to change input ninp of start
//static bool insert_copy_inst_after( Vec<InstAndPort> &assigned_ports, Inst *inst, Inst *start, int ninp, const char *reason ) {
//    PRINT( "AFTER" );
//    PRINT( reason );
//    return false;

//    _main_block->display_graphviz();

//    CC_SeqItemBlock *par_blk = start->cc_item_expr->parent_block;
//    Expr inp = start->inp[ ninp ];

//    // new Expr and CC_SeqItemExpr
//    Expr n_expr = copy( inp );
//    start->mod_inp( n_expr, ninp );
//    CC_SeqItemExpr *n_seqi = new CC_SeqItemExpr( n_expr, par_blk );
//    if ( not assign_port_rec( assigned_ports, n_expr.inst, -1, start->inp_reg[ ninp ] ) )
//         ERROR( "weird" );
//    ASSERT( n_expr->out_reg, "???" );


//    // position of start inst (inst will be inserted before, and in the same block)
//    int pos_start = 0;
//    while ( par_blk->seq[ pos_start ].ptr() != start->cc_item_expr )
//        ++pos_start;

//    // insertion in block
//    for( int i = pos_start - 1; ; --i ) {
//        if ( i < 0 or par_blk->seq[ i ]->contains( inst->cc_item_expr ) ) {
//            par_blk->seq.insert( i + 1, n_seqi );
//            break;
//        }
//    }

//    // always return false
//    return false;
//}

//// propagation through an edge
//struct RegPropagation : CC_SeqItem::Visitor {
//    // return true if OK, false if use of reg is forbidden (due to the fact that `ce` wants to do something else with this register)
//    virtual bool operator()( CC_SeqItemExpr &ce ) {
//        if ( port.is_an_output() ) { // -> going forward
//            // an instruction is going to produce something else, and it will be stored in reg
//            if ( ce.expr->out_reg == reg ) {
//                PRINT( *port.inst );
//                PRINT( ce.expr );
//                PRINT( *ce.expr->out_reg );
//                PRINT( *reg );
//                std::cout << std::endl;

//                return insert_copy_inst_before( *assigned_ports, ce.expr.inst, port.inst, "concurrent out reg" );
//            }

//            // an instruction want reg as input... but is not a target
//            int nb_same_reg_same_inp = 0;
//            int nb_same_reg_diff_inp = 0;
//            for( int i = 0; i < ce.expr->inp_reg.size(); ++i ) {
//                if ( reg == ce.expr->inp_reg[ i ] ) {
//                    if ( ce.expr->inp[ i ].inst == port.inst )
//                        ++nb_same_reg_same_inp;
//                    else
//                        ++nb_same_reg_diff_inp;
//                }
//            }
//            if ( nb_same_reg_diff_inp and not nb_same_reg_same_inp )
//                return insert_copy_inst_before( *assigned_ports, ce.expr.inst, port.inst, "concurrent inp reg" );
//        } else { // going backward
//            // an instruction was producing something that will be stored on out_reg (but is not the source inst)
//            if ( ce.expr->out_reg == reg )
//                return insert_copy_inst_after( *assigned_ports, ce.expr.inst, port.inst, port.ninp(), "out_reg used as output of another inst" );

//            // an instruction has reg as input... but is not pointing to the source inst
//            Inst *src = port.inst->inp[ port.ninp() ].inst;
//            int nb_same_reg_same_inp = 0;
//            int nb_same_reg_diff_inp = 0;
//            for( int i = 0; i < ce.expr->inp_reg.size(); ++i ) {
//                if ( reg == ce.expr->inp_reg[ i ] ) {
//                    if ( ce.expr->inp[ i ].inst == src )
//                        ++nb_same_reg_same_inp;
//                    else
//                        ++nb_same_reg_diff_inp;
//                }
//            }
//            if ( nb_same_reg_diff_inp and not nb_same_reg_same_inp ) {
//                PRINT( *port.inst );
//                PRINT( port.ninp() );
//                PRINT( *src );
//                PRINT( ce.expr );
//                PRINT( *reg );
//                std::cout << std::endl;
//                return insert_copy_inst_after( *assigned_ports, ce.expr.inst, port.inst, port.ninp(), "reg used as input of another inst" );
//            }
//        }

//        // forbid the use of reg for ce.expr (because there is an active living edge with reg)
//        ce.expr->reg_to_avoid.insert( reg );
//        return true;
//    }
//    Vec<InstAndPort> *assigned_ports;
//    CC_SeqItemExpr *inst_to_reach;
//    InstAndPort port;
//    CppOutReg *reg;
//};

void Codegen_C::write( Inst *inst ) {
    // reg decl
    std::map<String,Vec<CppOutReg *> > by_type;
    for( CppOutReg *r : inst->reg_to_decl )
        by_type[ type_to_str( r->type ) ] << r;
    for( auto it : by_type ) {
        on.write_beg() << it.first;
        for( int i = 0; i < it.second.size(); ++i )
            *os << ( i ? ", R" : " R" ) << it.second[ i ]->num;
        on.write_end( ";" );
    }

    // instruction
    inst->write( this );
}

static void update_created( Vec<Expr> &created, const Vec<Expr> &out ) {
    PI64 coi = ++Inst::cur_op_id;
    created.resize( 0 );
    for( Expr e : out )
        e->mark_children( &created );
    for( Expr &e : created )
        for( int i = 0; i < e->par.size(); ++i )
            if ( e->par[ i ].inst->op_id != coi )
                e->par.remove( i );
}

static bool insert_copy_inst_before( Vec<InstAndPort> &assigned_ports, Inst *beg, Inst *end ) {
    std::set<Inst *> avoid_par;
    for( Inst *inst = beg->next_sched; inst; inst = inst->next_sched ) {
        if ( inst == end ) {
            Expr cp = copy( beg );
            cp->when = new BoolOpSeq( True() );

            for( Inst::Parent &p : beg->par )
                if ( not avoid_par.count( p.inst ) )
                    p.inst->mod_inp( cp, p.ninp );

            assigned_ports << InstAndPort{ cp.inst, 0 };

            cp.inst->prev_sched = end->prev_sched;
            cp.inst->next_sched = end;
            end->prev_sched->next_sched = cp.inst;
            end->prev_sched = cp.inst;

            return false;
        }
        // update avoid par
        for( int ninp = 0; ninp < inst->inp.size(); ++ninp )
            if ( inst->inp[ ninp ] == beg )
                avoid_par.insert( inst );
    }
    return false;
}

void Codegen_C::make_code() {
    // a clone of the whole hierarchy
    ++Inst::cur_op_id;
    Vec<Expr> out, created;
    for( Expr inst : fresh )
        out << cloned( inst, created );

    // simplifications
    // e.g. ReplBits offset in bytes if possible
    // slices that do not change the size
    for( Expr &e : created )
        e->codegen_simplification( created, out );
    update_created( created, out );

    // display if necessary
    if ( disp_inst_graph )
        Inst::display_graph( out );

    // scheduling (creation of IfInst)
    Inst *beg = scheduling( out );
    update_created( created, out );

    // get reg constraints
    struct GetConstraint : Inst::Visitor {
        virtual bool operator()( Inst *inst ) {
            inst->get_constraints();
            return true;
        }
    };
    GetConstraint gc;
    beg->visit_sched( gc, true );

    //
    //    DisplayConstraints dv;
    //    beg->visit_sched( dv );

    //
    struct RegProgagation : Inst::Visitor {
        // return true if OK, false if use of reg is forbidden (due to the fact that `inst` wants to do something else with this register)
        virtual bool operator()( Inst *inst ) {
            // avoid the originating inst
            if ( inst == port.inst )
                return true;

            if ( port.is_an_output() ) {
                // an inst is going to produce something else in reg
                if ( inst->out_reg == reg )
                    return insert_copy_inst_before( *assigned_ports, port.inst, inst );

                //

            } else {
                TODO;
            }
            return true;
        }
        Vec<InstAndPort> *assigned_ports;
        Inst *inst_to_reach;
        InstAndPort port;
        CppOutReg *reg;
    };

    // assign (missing) out_reg
    struct AssignOutReg : Inst::Visitor {
        virtual bool operator()( Inst *inst ) {
            if ( inst->out_reg or not inst->need_a_register() )
                return true;

            // make a new reg
            CppOutReg *out_reg = cc->new_out_reg( inst->type() );

            // assign out_reg + constraint propagation ()
            Vec<InstAndPort> assigned_ports;
            if ( not assign_port_rec( assigned_ports, inst, -1, out_reg ) ) {
                std::cerr << "base constraint cannot be fullfilled\n";
                return false;
            }

            // edges and optionnal constraints
            int num_edge = 0; // , num_optionnal_constraint = 0;
            while ( true ) {
                // there's an edge for propagation ?
                if ( num_edge < assigned_ports.size() ) {
                    RegProgagation rp;
                    rp.assigned_ports = &assigned_ports;
                    rp.port = assigned_ports[ num_edge++ ];
                    if ( rp.port.is_an_output() ) {
                        Inst::Parent &p = rp.port.inst->par[ rp.port.nout() ];
                        rp.reg = rp.port.inst->out_reg;
                        rp.inst_to_reach = p.inst;
                        // RegPropagation may add a Copy inst and return false if inst is impossible to reach
                        if ( rp.port.inst->visit_sched( rp, true, true, rp.inst_to_reach ) ) {
                            if ( not assign_port_rec( assigned_ports, p.inst, p.ninp, rp.reg ) ) {
                                TODO;
                                // insert_copy_inst_before( assigned_ports, p.inst, rp.port.inst, "assignation not possible du to contraints on p.inst" );
                                return true;
                            }
                        }
                    } else {
                        CppOutReg *reg = rp.port.inst->inp_reg[ rp.port.ninp() ];
                        Inst *i = rp.port.inst->inp[ rp.port.ninp() ].inst;
                        assign_port_rec( assigned_ports, i, -1, reg );
                    }

                    //                    RegPropagation rp;
                    //                    rp.port = assigned_ports[ num_edge++ ];
                    //                    if ( rp.port.is_an_output() ) {
                    //                        Inst::Parent &p = rp.port.inst->par[ rp.port.nout() ];
                    //                        rp.inst_to_reach = p.inst->cc_item_expr;
                    //                        rp.reg = rp.port.inst->out_reg;
                    //                        // RegPropagation may add a Copy inst and return false if inst is impossible to reach
                    //                        if ( rp.port.inst->cc_item_expr->following_visit_to( rp, rp.inst_to_reach ) ) {
                    //                            // if not forbidden, assign input port of reached inst
                    //                            if ( not assign_port_rec( assigned_ports, p.inst, p.ninp, rp.reg ) ) {
                    //                                // if not possible to assign, add a copy inst
                    //                                insert_copy_inst_before( assigned_ports, p.inst, rp.port.inst, "assignation not possible du to contraints on p.inst" );
                    //                                return true;
                    //                            }
                    //                        }
                    //                    } else {
                    //                        Inst *i = rp.port.inst->inp[ rp.port.ninp() ].inst;
                    //                        rp.reg = rp.port.inst->inp_reg[ rp.port.ninp() ];
                    //                        rp.inst_to_reach = i->cc_item_expr;
                    //                        if ( rp.port.inst->cc_item_expr->preceding_visit_to( rp, rp.inst_to_reach ) ) {
                    //                            if ( not assign_port_rec( assigned_ports, i, -1, rp.reg ) ) {
                    //                                insert_copy_inst_after( assigned_ports, i, rp.port.inst, rp.port.ninp(), "pb assignment out reg" );
                    //                                return true;
                    //                            }
                    //                        }
                    //                    }
                    continue;
                }

                // constraint that would be good to fullfill
                //            if ( num_optionnal_constraint < assigned_ports.size() ) {
                //                InstAndPort port = assigned_ports[ num_optionnal_constraint++ ];

                //                for( std::pair<const Inst::PortConstraint,int> &c : port.inst->same_out )
                //                    if ( c.second != Inst::COMPULSORY and c.first.src_ninp == port.ninp_constraint() )
                //                        assign_port_rec( assigned_ports, c.first.dst_inst, c.first.dst_ninp, out_reg );

                //                continue;
                //            }


                // nothing to propagate
                break;
            }
            return true;
        }
        Codegen_C *cc;
    };
    AssignOutReg aor; aor.cc = this;
    beg->visit_sched( aor, true );

    // display if necessary
    if ( disp_inst_graph_wo_phi )
        Inst::display_graph( out );

    // specify where the registers have to be declared
    for( int n = 0; n < out_regs.size(); ++n ) {
        CppOutReg *reg = &out_regs[ n ];
        Inst *anc = reg->common_provenance_ancestor();
        anc->reg_to_decl << reg;
    }

    // write the code
    for( Inst *inst = beg; inst; inst = inst->next_sched )
        write( inst );
}


