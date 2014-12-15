/****************************************************************************
**
** Copyright (C) 2014 SocaDB
**
** This file is part of the SocaDB toolkit/database.
**
** SocaDB is free software. You can redistribute this file and/or modify
** it under the terms of the Apache License, Version 2.0 (the "License").
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
** You should have received a copy of the Apache License, Version 2.0
** along with this program. If not, see
** <http://www.apache.org/licenses/LICENSE-2.0.html>.
**
**
** Commercial License Usage
**
** Alternatively, licensees holding valid commercial SocaDB licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and SocaDB.
**
**
****************************************************************************/

#ifndef INST_H
#define INST_H

#include "../System/Vec.h"
#include "Expr.h"

/**
*/
class Inst {
public:
    struct Parent {
        bool operator==( const Parent &p ) const { return inst == p.inst and ninp == p.ninp; }
        bool operator< ( const Parent &p ) const { return inst != p.inst ? inst < p.inst : ninp < p.ninp; }
        Inst *inst;
        int   ninp; ///< input number (or TPAR_...)
    };
    /// flags
    enum {
        CONST     = 1,
        SURDEF    = 2
    };
    /// for Parent::ninp
    enum {
        TPAR_DEP = -1
    };
    ///
    struct Visitor {
        virtual bool operator()( Inst *inst ) = 0;
    };

    Inst();
    virtual ~Inst();

    virtual bool same_cst( const Inst *inst ) const;
    virtual bool emas_cst( const Inst *inst ) const;

    void add_dep( const Expr &val );
    void add_inp( const Expr &val );
    void add_ext( const Expr &val );

    void mod_inp( const Expr &val, int num );
    void mod_dep( const Expr &val, Inst *d );

    void rem_dep( Inst *d );
    void rem_ref_to_this();

    void add_store_dep( Inst *dst );

    void mark_children( Vec<Expr> *seq );

    void clone( Vec<Expr> &created ) const;
    virtual Expr forced_clone( Vec<Expr> &created ) const = 0;

    virtual void write_to_stream( Stream &os, int prec = -1 );
    virtual void write_dot( Stream &os ) const = 0;

    virtual void _mk_store_dep( Inst *dst );

    // display
    static  int  display_graph( Vec<Expr> outputs, const char *filename = ".res" );
    virtual void write_graph_rec( Vec<Inst *> &ext_buf, Vec<Inst *> &seq, Stream &os );
    virtual void write_sub_graph_rec( Vec<Inst *> &seq, Stream &os );
    virtual int  ext_disp_size() const;

    Vec<Expr>             inp;
    Vec<Expr>             ext;
    Vec<Expr>             dep;
    mutable Vec<Parent>   par; ///< parents
    mutable Inst         *ext_par;
    int                   flags;

    static  PI64          cur_op_id; ///<
    mutable PI64          op_id_vis; ///<
    mutable PI64          op_id;     ///< operation id (every new operation on the graph begins with ++current_MO_op_id and one can compare op_id with cur_op_id to see if operation on this node has been done or not).
    mutable void         *op_mp;     ///< result of current operations
    mutable int           cpt_use;   ///< garbage collection
};

#endif // INST_H
