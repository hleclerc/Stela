#include "CC_SeqItem.h"
#include <fstream>

PI64 CC_SeqItem::cur_op_id = 0;

CC_SeqItem::CC_SeqItem( CC_SeqItem *parent, CC_SeqItemBlock *parent_block ) : parent_block( parent_block ), parent( parent ) {
    op_id = 0;
}

CC_SeqItem::~CC_SeqItem() {
}

void CC_SeqItem::get_glo_cond_and_seq_of_sub_blocks( Vec<CC_SeqItemBlock *> &res, const BoolOpSeq &cond ) {
}

bool CC_SeqItem::following_visit( Visitor &v, CC_SeqItem *avoid ) {
    return parent ? parent->following_visit( v, this ) : true;
}

bool CC_SeqItem::preceding_visit( Visitor &v, CC_SeqItem *avoid ) {
    return parent ? parent->preceding_visit( v, this ) : true;
}

bool CC_SeqItem::contains( CC_SeqItemExpr *expr ) {
    struct Contains : Visitor {
        virtual bool operator()( CC_SeqItemExpr &ce ) {
            if ( &ce != expr )
                return true;
            found = true;
            return false;
        }
        CC_SeqItemExpr *expr;
        bool found;
    };
    Contains c;
    c.expr = expr;
    c.found = false;
    visit( c, true );
    return c.found;
}

bool CC_SeqItem::visit( Visitor &v, bool forward ) {
    return true;
}

int CC_SeqItem::display_graphviz() {
    std::string filename = ".res.dot";
    std::ofstream f( filename );
    f << "digraph Instruction {\n";
    // f << "  node [shape=plaintext];\n";
    f << "  graph [rankdir=LR];\n";

    int level = 0;
    write_graphviz( f, level );

    for( int i = 0; i <= level; ++i )
        f << i << " [style=\"invis\"];\n";
    for( int i = 1; i <= level; ++i )
        f << i - 1 << " -> " << i << " [style=\"invis\"];\n";

    f << "}";
    f.close();

    return system( ( "dot -Tps " + std::string( filename ) + " > " + std::string( filename ) + ".eps && gscv " + std::string( filename ) + ".eps" ).c_str() );
}
