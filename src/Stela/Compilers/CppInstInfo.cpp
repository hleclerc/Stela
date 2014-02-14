#include "CppInstCompiler.h"
#include "CppInstInfo.h"

void CppInstInfo::DeclWriter::write_to_stream( Stream &os ) const {
    const BaseType *bt = info->out[ nout ].type;
    ASSERT( info->out[ nout ].num < 0, "weird" );
    ASSERT( bt, "bad" );

    info->out[ nout ].num = cc->get_free_reg( bt );
    os << *bt << " R" << info->out[ nout ].num << " = ";
}

void CppInstInfo::InstWriter::write_to_stream( Stream &os ) const {
    if ( info->out[ nout ].num >= 0 )
        os << "R" << info->out[ nout ].num;
    else {
        CppInstCompiler cic( cc, true );
        info->inst->apply( cic );
    }
}

CppInstInfo::DeclWriter CppInstInfo::decl_writer( CppCompiler *cc, int nout ) {
    DeclWriter res;
    res.info = this;
    res.nout = nout;
    res.cc = cc;
    return res;
}

CppInstInfo::InstWriter CppInstInfo::inst_writer( CppCompiler *cc, int nout ) {
    InstWriter res;
    res.info = this;
    res.nout = nout;
    res.cc = cc;
    return res;
}
