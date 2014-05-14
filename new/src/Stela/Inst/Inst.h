#ifndef INST_H
#define INST_H

#include "../System/Stream.h"
#include "../System/Ptr.h"
#include "../System/Vec.h"
#include <string.h>
class CodeGen_C;
class Type;
class Var;

/**
*/
class Inst : public ObjectWithCptUse {
public:
    struct Parent {
        bool operator==( const Parent &p ) { return inst == p.inst and ninp == p.ninp; }
        Inst *inst;
        int   ninp; ///< input number
    };
    struct Visitor {
        virtual void operator()( Inst *inst ) = 0;
    };

    Inst();
    virtual ~Inst();
    virtual void write_to_stream( Stream &os ) const;
    virtual void write_dot( Stream &os ) const = 0;
    virtual int size() const = 0;

    bool true_if( const ConstPtr<Inst> &cond ) const; ///< return true if this will be true if cond is checked
    bool rtrue_if( const ConstPtr<Inst> &val ) const; ///< commuted true if (this is the cond)

    virtual void set( Ptr<Inst> val );
    virtual void add_var_ptr( Var *var );

    void add_inp( Ptr<Inst> val );
    void mod_inp( int num, Ptr<Inst> val );
    void add_dep( Ptr<Inst> val );

    virtual void clone( Vec<Ptr<Inst> > &created ) const; ///< output in op_mp
    virtual Ptr<Inst> forced_clone( Vec<Ptr<Inst> > &created ) const = 0;

    void rec_visit( Visitor &visitor, bool want_ext = false ); ///< ++Inst::cur_op_id to be done

    template<class T>
    bool get_val( T &res ) {
        if ( const PI8 *ptr = data_ptr() ) {
            int sb = ( size() + 7 ) / 8;
            if ( sizeof( res ) <= sb ) {
                memcpy( &res, ptr, sizeof( res ) );
                return true;
            }
        }
        return false;
    }

    virtual const PI8 *data_ptr( int offset = 0 ) const;
    virtual bool is_and() const; ///< hum

    virtual Ptr<Inst> snapshot();
    virtual void write_to( CodeGen_C *cc ) const;
    virtual void write_1l_to( CodeGen_C *cc ) const = 0;
    virtual Type *out_type_proposition( CodeGen_C *cc ) const;
    virtual Type *inp_type_proposition( CodeGen_C *cc, int ninp ) const;

    virtual void add_when_cond( const Ptr<Inst> &cond ); ///< or cond to IIC( this )->when

    // graphviz
    static int display_graph( const Vec<ConstPtr<Inst> > &outputs, const char *filename = ".res" );
    virtual void write_graph_rec( Vec<const Inst *> &ext_buf, Stream &os ) const;
    virtual void write_sub_graph_rec( Stream &os ) const;

    //
    virtual Ptr<Inst> _simplified();
    virtual Ptr<Inst> _pointer_on( int beg, int len );
    virtual void _remove_cond( Vec<Ptr<Inst> > &cr );

    // parameters
    Vec<Ptr<Inst> > inp; ///< inputs
    Vec<Ptr<Inst> > dep; ///< dependencies
    Vec<Ptr<Inst> > ext; ///< for WhileOut, IfOut, ...
    Vec<Ptr<Inst> > exi; ///< for WhileInp, ...
    Vec<Parent>     par; ///> parents
    Inst           *ext_par;

    //
    static  PI64  cur_op_id; ///<
    mutable PI64  op_id_vis; ///<
    mutable PI64  op_id;     ///< operation id (every new operation on the graph begins with ++current_MO_op_id and one can compare op_id with cur_op_id to see if operation on this node has been done or not).
    mutable void *op_mp;     ///< result of current operations
};

/// uses ip for simplifications
Ptr<Inst> simplified( Ptr<Inst> val );

#endif // INST_H
