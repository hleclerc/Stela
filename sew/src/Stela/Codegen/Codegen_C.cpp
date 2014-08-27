#include "../System/AutoPtr.h"
#include "../Inst/BoolOpSeq.h"
#include "../Inst/Type.h"
#include "../Inst/Copy.h"
#include "../Inst/Ip.h"
#include "CC_SeqItemExpr.h"
#include "CC_SeqItemIf.h"
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

static BoolOpSeq anded( const Vec<BoolOpSeq> &cond_stack ) {
    BoolOpSeq res;
    for( const BoolOpSeq &b : cond_stack )
        res = res and b;
    return res;
}

void Codegen_C::scheduling( CC_SeqItemBlock *cur_block, Vec<Expr> out ) {
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
    Vec<CC_SeqItemExpr *> seq_expr;
    Vec<BoolOpSeq> cond_stack;
    cond_stack << BoolOpSeq();
    ++Inst::cur_op_id;
    while ( front.size() ) {
        Expr inst;
        // try to find an instruction with the same condition set or an inst that is not going to write anything
        BoolOpSeq cur_cond = anded( cond_stack );
        for( int i = 0; i < front.size(); ++i ) {
            if ( *front[ i ]->when == cur_cond ) {
                inst = front[ i ];
                front.remove_unordered( i );
                break;
            }
        }
        // else, try to find an instruction with few changes on conditions
        if ( not inst ) {
            int best_score = std::numeric_limits<int>::max(), best_index = -1;
            int best_nb_close = 0, best_nb_else = 0;
            Vec<BoolOpSeq> best_cond_stack;
            BoolOpSeq best_nc;

            for( int i = 0; i < front.size(); ++i ) {
                BoolOpSeq &cond = *front[ i ]->when;
                Vec<BoolOpSeq> trial_cond_stack = cond_stack;
                int nb_close = 0, nb_else = 0;

                BoolOpSeq poped; // the last cond
                if ( cond.always( true ) ) {
                    nb_close = trial_cond_stack.size() - 1;
                    trial_cond_stack.resize( 1 );
                } else {
                    while ( not cond.imply( anded( trial_cond_stack ) ) ) {
                        poped = trial_cond_stack.pop_back_val();
                        if ( poped.or_seq.size() )
                            ++nb_close;
                    }
                }
                BoolOpSeq nc = cond - anded( trial_cond_stack );

                if ( nc.imply( not poped ) ) {
                    trial_cond_stack << not poped;
                    nc = nc - not poped;
                    --nb_close;
                    ++nb_else;
                }
                if ( nc.or_seq.size() )
                    trial_cond_stack << nc;

                int score = nb_close * 10000 + nb_else + 2 * nc.nb_sub_conds();
                if ( best_score > score ) {
                    best_cond_stack = trial_cond_stack;
                    best_nb_close = nb_close;
                    best_nb_else = nb_else;
                    best_score = score;
                    best_index = i;
                    best_nc = nc;
                }
            }
            inst = front[ best_index ];
            cond_stack = best_cond_stack;
            front.remove_unordered( best_index );

            for( ; best_nb_close; --best_nb_close )
                cur_block = cur_block->parent_block;
            if ( best_nb_else )
                cur_block = cur_block->sibling;
            if ( best_nc.or_seq.size() ) {
                CC_SeqItemIf *new_block = new CC_SeqItemIf( cur_block );
                cur_block->seq << new_block;
                new_block->cond = best_nc;
                cur_block = &new_block->seq[ 0 ];
            }
        }

        // the new CC_SeqItemExpr
        CC_SeqItemExpr *ne = new CC_SeqItemExpr( inst, cur_block );
        inst->op_id = Inst::cur_op_id; // say that it's done
        cur_cond = *inst->when;
        cur_block->seq << ne;
        seq_expr << ne;

        // parents
        for( Inst::Parent &p : inst->par ) {
            if ( ready_to_be_scheduled( p.inst ) ) {
                p.inst->op_id = Inst::cur_op_id - 1; // -> in the front
                front << p.inst;
            }
        }
    }

    // delayed operation (ext blocks)
    for( CC_SeqItemExpr *ne : seq_expr ) {
        // schedule sub block (ext instructions)
        int s = ne->expr->ext_disp_size();
        ne->ext.resize( s );
        for( int e = 0; e < s; ++e ) {
            CC_SeqItemBlock *b = new CC_SeqItemBlock;
            b->parent_block = ne->parent_block;
            ne->ext[ e ] = b;

            scheduling( b, ne->expr->ext[ e ] );
        }

        // add internal break or continue if necessary
        CC_SeqItemBlock *b[ s ];
        for( int i = 0; i < s; ++i )
            b[ i ] = ne->ext[ i ].ptr();
        ne->expr->add_break_and_continue_internal( b );
    }


}

struct GetConstraint : CC_SeqItem::Visitor {
    virtual bool operator()( CC_SeqItemExpr &ce ) {
        ce.expr->get_constraints();
        for( AutoPtr<CC_SeqItemBlock> &ext : ce.ext )
            ext->visit( *this, true );
        return true;
    }
};

struct DisplayConstraints : CC_SeqItem::Visitor {
    virtual bool operator()( CC_SeqItemExpr &ce ) {
        Inst *i = ce.expr.inst;
        i->write_dot( std::cout );
        std::cout << " ->\n";
        for( auto o : i->same_out ) {
            if ( o.first.src_ninp < 0 )
                std::cout << "  out ";
            else
                std::cout << "  [" << o.first.src_ninp << "] ";
            o.first.dst_inst->write_dot( std::cout << "== " );
            if ( o.first.dst_ninp < 0 )
                std::cout << " out ";
            else
                std::cout << "[" << o.first.dst_ninp << "] ";
            std::cout << "\n";
        }
        for( auto o : i->diff_out ) {
            if ( o.first.src_ninp < 0 )
                std::cout << "  out ";
            else
                std::cout << "  [" << o.first.src_ninp << "] ";
            o.first.dst_inst->write_dot( std::cout << "!= " );
            if ( o.first.dst_ninp < 0 )
                std::cout << " out ";
            else
                std::cout << "[" << o.first.dst_ninp << "] ";
            std::cout << "\n";
        }
        return true;
    }
};

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
    // look if cutting an validated edge using out_reg
    if ( inst->reg_to_avoid.count( out_reg ) )
        return false;

    // assignation
    if ( ninp < 0 ) { // output ?
        if ( inst->out_reg )
            return inst->out_reg == out_reg;
        inst->out_reg = out_reg;
        out_reg->provenance << inst->cc_item_expr;
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

static bool insert_copy_inst_before( Vec<InstAndPort> &assigned_ports, Inst *inst, Inst *start ) {
    CC_SeqItemBlock *par_blk = start->cc_item_expr->parent_block;

    // new Expr and CC_SeqItemExpr
    Expr n_expr = copy( start );
    CC_SeqItemExpr *n_seqi = new CC_SeqItemExpr( n_expr, par_blk );
    if ( not assign_port_rec( assigned_ports, n_expr.inst, 0, start->out_reg ) )
         ERROR( "weird" );

    // position of start inst (inst will be inserted after, and in the same block)
    int pos_start = 0;
    while ( par_blk->seq[ pos_start ].ptr() != start->cc_item_expr )
        ++pos_start;

    // insertion in block
    for( int i = pos_start + 1; ; ++i ) {
        // we're at the end of the block
        if ( i == par_blk->seq.size() ) {
            std::cout << "    bef1 " << *start  << " inst=" << *inst << "\n";
            par_blk->seq << n_seqi;
            break;
        }

        // modify input of instructions that use start
//        struct ModInp : CC_SeqItem::Visitor {
//            virtual bool operator()( CC_SeqItemExpr &ce ) {
//                for( int ninp = 0; ninp < ce.expr->inp.size(); ++ninp )
//                    if ( ce.expr->inp[ ninp ].inst == oexpr )
//                        ce.expr->mod_inp( nexpr, ninp );
//                return true;
//            }

//            Inst *nexpr;
//            Inst *oexpr;
//        };

//        ModInp mi;
//        mi.oexpr = start;
//        mi.nexpr = n_expr.inst;
//        par_blk->seq[ i ]->visit( mi, true );

        //
        if ( par_blk->seq[ i ]->contains( inst->cc_item_expr ) ) {
            std::cout << "    before2 start=" << *start  << " inst=" << *inst << "\n";
            par_blk->seq.insert( i, n_seqi );
            break;
        }
    }

    // always return false (for convenience)
    return false;
}

/// to change input ninp of start
static bool insert_copy_inst_after( Vec<InstAndPort> &assigned_ports, Inst *inst, Inst *start, int ninp, const char *reason ) {
    TODO;

    CC_SeqItemBlock *par_blk = start->cc_item_expr->parent_block;
    Expr inp = start->inp[ ninp ];

    // new Expr and CC_SeqItemExpr
    Expr n_expr = copy( inp );
    start->mod_inp( n_expr, ninp );
    CC_SeqItemExpr *n_seqi = new CC_SeqItemExpr( n_expr, par_blk );
    if ( not assign_port_rec( assigned_ports, n_expr.inst, -1, start->inp_reg[ ninp ] ) )
         ERROR( "weird" );
    ASSERT( n_expr->out_reg, "???" );


    // position of start inst (inst will be inserted before, and in the same block)
    int pos_start = 0;
    while ( par_blk->seq[ pos_start ].ptr() != start->cc_item_expr )
        ++pos_start;

    // insertion in block
    for( int i = pos_start - 1; ; --i ) {
        if ( i < 0 or par_blk->seq[ i ]->contains( inst->cc_item_expr ) ) {
            std::cout << "    after " << *inst << " insert copy of " << *start << "[" << ninp << "] reason=" << reason << "\n";
            par_blk->seq.insert( i + 1, n_seqi );
            break;
        }
    }

    // always return false
    return false;
}

// propagation through an edge
struct RegPropagation : CC_SeqItem::Visitor {
    // return true if OK, false if use of reg is forbidden (due to ce that wants to do something with this register)
    virtual bool operator()( CC_SeqItemExpr &ce ) {
        if ( port.is_an_output() ) { // -> going forward
            // an instruction is going to produce something that will be stored in reg
            if ( ce.expr->out_reg == reg )
                return insert_copy_inst_before( *assigned_ports, ce.expr.inst, port.inst );

            // an instruction want reg as input... but is not a target
            for( int i = 0; i < ce.expr->inp_reg.size(); ++i )
                if ( reg == ce.expr->inp_reg[ i ] and ce.expr->inp[ i ].inst != port.inst )
                    return insert_copy_inst_before( *assigned_ports, ce.expr.inst, port.inst );
        } else { // going backward
            // an instruction was producing something that will be stored on out_reg (but is not the source inst)
            if ( ce.expr->out_reg == reg )
                return insert_copy_inst_after( *assigned_ports, ce.expr.inst, port.inst, port.ninp(), "out_reg used as output of another inst" );

            // an instruction has reg as input... but is not pointing to the source inst
            Inst *src = port.inst->inp[ port.ninp() ].inst;
            for( int i = 0; i < ce.expr->inp_reg.size(); ++i )
                if ( reg == ce.expr->inp_reg[ i ] and ce.expr->inp[ i ].inst != src )
                    return insert_copy_inst_after( *assigned_ports, ce.expr.inst, port.inst, port.ninp(), "reg used as input of another inst" );
        }

        // forbid the use of reg for ce.expr (because there is an active living edge with reg)
        ce.expr->reg_to_avoid.insert( reg );
        return true;
    }
    Vec<InstAndPort> *assigned_ports;
    CC_SeqItemExpr *inst_to_reach;
    InstAndPort port;
    CppOutReg *reg;
};

struct AssignOutReg : CC_SeqItem::Visitor {
    virtual bool operator()( CC_SeqItemExpr &ce ) {
        if ( ce.expr->out_reg or not ce.expr->need_a_register() )
            return true;

        // make a new reg
        CppOutReg *out_reg = cc->new_out_reg( ce.expr->type() );

        // assign out_reg + constraint propagation ()
        Vec<InstAndPort> assigned_ports;
        if ( not assign_port_rec( assigned_ports, ce.expr.inst, -1, out_reg ) ) {
            std::cerr << "base constraint cannot be fullfilled\n";
            return false;
        }

        // edges and optionnal constraints
        int num_edge = 0, num_optionnal_constraint = 0;
        while ( true ) {
            // edge propagation (priority)
            if ( num_edge < assigned_ports.size() ) {
                RegPropagation rp;
                rp.assigned_ports = &assigned_ports;
                rp.port = assigned_ports[ num_edge++ ];
                if ( rp.port.is_an_output() ) {
                    Inst::Parent &p = rp.port.inst->par[ rp.port.nout() ];
                    rp.inst_to_reach = p.inst->cc_item_expr;
                    rp.reg = rp.port.inst->out_reg;
                    //
                    if ( rp.port.inst->cc_item_expr->following_visit_to( rp, rp.inst_to_reach ) )
                        // if not forbidden, assign port
                        if ( not assign_port_rec( assigned_ports, p.inst, p.ninp, rp.reg ) )
                            // if not possible to assign, add a copy inst
                            return insert_copy_inst_before( assigned_ports, p.inst, rp.port.inst );
                } else {
                    Inst *i = rp.port.inst->inp[ rp.port.ninp() ].inst;
                    rp.reg = rp.port.inst->inp_reg[ rp.port.ninp() ];
                    rp.inst_to_reach = i->cc_item_expr;
                    if ( rp.port.inst->cc_item_expr->preceding_visit_to( rp, rp.inst_to_reach ) )
                        if ( not assign_port_rec( assigned_ports, i, -1, rp.reg ) )
                            return insert_copy_inst_after( assigned_ports, i, rp.port.inst, rp.port.ninp(), "pb assignment out reg" );
                }
                continue;
            }

            // constraint that would be good to fullfill
            if ( num_optionnal_constraint < assigned_ports.size() ) {
                InstAndPort port = assigned_ports[ num_optionnal_constraint++ ];

                for( std::pair<const Inst::PortConstraint,int> &c : port.inst->same_out )
                    if ( c.second != Inst::COMPULSORY and c.first.src_ninp == port.ninp_constraint() )
                        assign_port_rec( assigned_ports, c.first.dst_inst, c.first.dst_ninp, out_reg );

                continue;
            }


            // nothing to propagate
            break;
        }
        return true;
    }
    Codegen_C *cc;
};

void Codegen_C::make_code() {
    // a clone of the whole hierarchy
    ++Inst::cur_op_id;
    Vec<Expr> out, created;
    for( Expr inst : fresh )
        out << cloned( inst, created );

    // simplifications (+ insert Copy before Select, ...)
    for( Expr &e : created )
        e->codegen_simplification( created, out );

    // remove parents that are not part of the graph (after the simplifications)
    // + update `created`
    PI64 coi = ++Inst::cur_op_id;
    created.resize( 0 );
    for( Expr &e : out )
        e->mark_children( &created );
    for( Expr &e : created )
        for( int i = 0; i < e->par.size(); ++i )
            if ( e->par[ i ].inst->op_id != coi )
                e->par.remove( i );

    // display
    // if ( disp_inst_graph )
    //     Inst::display_graph( out );

    // scheduling
    CC_SeqItemBlock main_block;
    scheduling( &main_block, out );

    // get reg constraints
    GetConstraint gc;
    main_block.visit( gc, true );

    // DisplayConstraints dv;
    // main_block.visit( dv );

    // assign (missing) out_reg
    AssignOutReg aor; aor.cc = this;
    main_block.visit( aor, true );

    // display
    if ( disp_inst_graph )
        Inst::display_graph( out );

    // specify where the registers have to be declared
    for( int n = 0; n < out_regs.size(); ++n ) {
        CppOutReg &reg = out_regs[ n ];
        CC_SeqItemBlock *anc = reg.common_provenance_ancestor();
        anc->reg_to_decl << &reg;
    }

    // write the code
    main_block.write( this );
}


