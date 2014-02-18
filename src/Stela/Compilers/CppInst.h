#ifndef CPPINST_H
#define CPPINST_H

#include "../System/Stream.h"
#include "../System/Vec.h"
#include "CppExpr.h"
class CppCompiler;
class BaseType;

/**
*/
class CppInst {
public:
    struct Out {
        struct Parent {
            bool operator==( const Parent &item ) const { return inst == item.inst and ninp == item.ninp; }
            CppInst *inst;
            int      ninp;
        };
        Out() : bt_hint( 0 ), num( -1 ) {}

        mutable const BaseType *bt_hint;
        Vec<Parent,-1,1> parents;
        int num; ///< num register
    };

    struct GenWriter {
        mutable CppCompiler *cc;
        mutable CppInst *info;
        int nout;
    };

    struct DeclWriter : GenWriter {
        void write_to_stream( Stream &os ) const;
    };

    struct InstWriter : GenWriter {
        void write_to_stream( Stream &os ) const;
        int precedance;
    };

    CppInst( int inst_id, int nb_outputs );

    void add_child( CppExpr expr );
    void update_bt_hints() const;

    const BaseType *get_bt_hint_for_nout( int nout ) const; ///< helper for update_bt_hints()
    const BaseType *inp_bt_hint( int ninp ) const;
    const BaseType *out_bt_hint( int nout ) const;

    DeclWriter decl( CppCompiler *cc, int nout );
    InstWriter inst( CppCompiler *cc, int nout, int precedance = 0 );
    void write_to_stream( Stream &os ) const;
    void write_code( CppCompiler *cc, int prec = -1 );

    static int display_graph( const Vec<CppInst *> &res, const char *filename = ".res" );
    void write_graph_rec( Stream &os ) const;

    void write_code_bin_op( CppCompiler *cc, int prec, const char *op_str, int prec_op );

    // attributes
    PI8              *additionnal_data; ///< e.g. value for Cst
    int               inst_id;
    Vec<Out    ,-1,1> out;
    Vec<CppExpr,-1,2> inp;

    mutable PI64      op_id;     ///< operation id (every new operation on the graph begins with ++current_MO_op_id and one can compare op_id with cur_op_id to see if operation on this node has been done or not).
    mutable void     *op_mp;     ///< result of current operations
    mutable PI64      op_id_vis; ///<
    static  PI64      cur_op_id; ///<
};


#endif // CPPINST_H
