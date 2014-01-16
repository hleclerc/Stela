#ifndef CPPCOMPILER_H
#define CPPCOMPILER_H

#include "../Interpreter/BaseType.h"
#include "../System/SplittedVec.h"
#include "../System/StreamSep.h"
#include "../Interpreter/Expr.h"
#include <sstream>

/**
*/
class CppCompiler {
public:
    CppCompiler();

    CppCompiler &operator<<( ConstPtr<Inst> inst );
    void exec();

    String cpp_filename;
protected:
    friend class CppInstCompiler;

    void get_front_rec( Vec<const Inst *> &front, const Inst *inst );
    void compile();

    struct Reg {
        Reg() : type( 0 ), num( -1 ) {}
        operator bool() const { return type; }
        void write_to_stream( Stream &os ) const { os << 'R' << num; }
        Stream &write_decl( Stream &os ) const {
            if ( type )
                os << *type << " R" << num << " = ";
            else
                os << "[ no base type ]";
            return os;
        }
        const BaseType *type;
        int num;
    };

    struct Info {
        Info( const Inst *inst ) : inst( inst ) {
        }
        Reg get_out_reg( int nout ) {
            return nout < _out_regs.size() ? _out_regs[ nout ] : Reg();
        }
        Reg get_inp_reg( int ninp ) {
            Expr expr = inst->inp_expr( ninp );
            return reinterpret_cast<Info *>( expr.inst->op_mp )->get_out_reg( expr.nout );
        }
        void set_out_reg( int nout, Reg reg ) {
            if ( nout >= _out_regs.size() )
                _out_regs.resize( nout + 1 );
            _out_regs[ nout ] = reg;
        }
        Vec<Reg> _out_regs;
        const Inst *inst;
    };

    Reg get_reg_for(const Inst &inst, int nout );


    SplittedVec<Info,64> info_it;
    Vec<ConstPtr<Inst> > outputs;
    int nb_regs;

    StreamSepMaker<std::ostringstream> on;
    std::ostringstream os;
};

#endif // CPPCOMPILER_H
